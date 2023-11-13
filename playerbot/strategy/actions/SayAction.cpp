#include "botpch.h"
#include "../../playerbot.h"
#include "SayAction.h"
#include "../../PlayerbotTextMgr.h"
#include "ChannelMgr.h"
#include "../../ServerFacade.h"
#include <regex>
/*
// GPT Experiment
#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <array>
// GPT Experiment
*/
#include <iostream>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

using namespace ai;

std::unordered_set<std::string> noReplyMsgs = {
  "join", "leave", "follow", "attack", "pull", "flee", "reset", "reset ai",
  "all ?", "talents", "talents list", "talents auto", "talk", "stay", "stats",
  "who", "items", "leave", "join", "repair", "summon", "nc ?", "co ?", "de ?",
  "dead ?", "follow", "los", "guard", "do accept invitation", "stats", "react ?", 
  "reset strats", "home",
};
std::unordered_set<std::string> noReplyMsgParts = { "+", "-", "follow target", "focus heal", "cast ", "accept [", "e [", "destroy [", "go zone"};

std::unordered_set<std::string> noReplyMsgStarts = { "e ", "accept ", "cast ", "destroy "};

static string lastReplyMsg = "";

SayAction::SayAction(PlayerbotAI* ai) : Action(ai, "say"), Qualified()
{
}

bool SayAction::Execute(Event& event)
{
    string text = "";
    map<string, string> placeholders;
    Unit* target = AI_VALUE(Unit*, "tank target");
    if (!target) target = AI_VALUE(Unit*, "current target");

    // set replace strings
    if (target) placeholders["<target>"] = target->GetName();
    placeholders["<randomfaction>"] = IsAlliance(bot->getRace()) ? "Alliance" : "Horde";
    if (qualifier == "low ammo" || qualifier == "no ammo")
    {
        Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
        if (pItem)
        {
            switch (pItem->GetProto()->SubClass)
            {
            case ITEM_SUBCLASS_WEAPON_GUN:
                placeholders["<ammo>"] = "bullets";
                break;
            case ITEM_SUBCLASS_WEAPON_BOW:
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                placeholders["<ammo>"] = "arrows";
                break;
            }
        }
    }

    if (bot->IsInWorld())
    {
        if (AreaTableEntry const* area = GetAreaEntryByAreaID(sServerFacade.GetAreaId(bot)))
            placeholders["<subzone>"] = area->area_name[0];
    }

    // set delay before next say
    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    uint32 nextTime = time(0) + urand(1, 30);
    ai->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime);

    Group* group = bot->GetGroup();
    if (group)
    {
        vector<Player*> members;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->getSource();
            PlayerbotAI* memberAi = member->GetPlayerbotAI();
            if (memberAi) members.push_back(member);
        }

        uint32 count = members.size();
        if (count > 1)
        {
            for (uint32 i = 0; i < count * 5; i++)
            {
                int i1 = urand(0, count - 1);
                int i2 = urand(0, count - 1);

                Player* item = members[i1];
                members[i1] = members[i2];
                members[i2] = item;
            }
        }

        int index = 0;
        for (vector<Player*>::iterator i = members.begin(); i != members.end(); ++i)
        {
            PlayerbotAI* memberAi = (*i)->GetPlayerbotAI();
            if (memberAi)
                memberAi->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime + (20 * ++index) + urand(1, 15));
        }
    }

    // load text based on chance
    if (!sPlayerbotTextMgr.GetBotText(qualifier, text, placeholders))
        return false;

    if (text.find("/y ") == 0)
        bot->Yell(text.substr(3), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
    else
        bot->Say(text, (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));

    return true;
}


bool SayAction::isUseful()
{
    if (!ai->AllowActivity())
        return false;

    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(0) - lastSaid) > 30;
}

size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* response = static_cast<std::string*>(userdata);
    size_t realSize = size * nmemb;
    response->append(ptr, realSize);
    return realSize;
}

void ChatReplyAction::ChatReplyDo(Player* bot, uint32 type, uint32 guid1, uint32 guid2, std::string msg, std::string chanName, std::string name)
{
    ChatReplyType replyType = REPLY_NOT_UNDERSTAND; // default not understand
    std::string respondsText = "";

    PlayerbotAI* ai = bot->GetPlayerbotAI();
    Player* bMaster;
    uint32 masterID;

    Group* group = bot->GetGroup();
    if (group)
    {
        Player* bMaster = ai->GetGroupMaster();
        uint32 masterID = bMaster->GetGUIDLow();
    }

    uint32 botID;

    try
    {
        botID = bot->GetGUIDLow(); // crash?
    }
    catch (exception e)
    {
        return;
    }

    std::string botName = bot->GetName();


    // if we're just commanding bots around, don't respond...
    // first one is for exact word matches
    if (noReplyMsgs.find(msg) != noReplyMsgs.end()) {
        ostringstream out;
        out << "DEBUG ChatReplyDo decided to ignore exact blocklist match" << msg;
        //bot->Say(out.str(), LANG_UNIVERSAL);
        return;
    }

    // second one is for partial matches like + or - where we change strats
    if (std::any_of(noReplyMsgParts.begin(), noReplyMsgParts.end(), [&msg](const std::string& part) { return msg.find(part) != std::string::npos; })) {
        ostringstream out;
        out << "DEBUG ChatReplyDo decided to ignore partial blocklist match" << msg;
        //bot->Say(out.str(), LANG_UNIVERSAL);

        return;
    }

    if (std::any_of(noReplyMsgStarts.begin(), noReplyMsgStarts.end(), [&msg](const std::string& start) {
        return msg.find(start) == 0;  // Check if the start matches the beginning of msg
        })) {
        ostringstream out;
        out << "DEBUG ChatReplyDo decided to ignore start blocklist match" << msg;
        //bot->Say(out.str(), LANG_UNIVERSAL);
        return;
    }

    // check if msg has already been replied to by somebody else 
    // ? maybe let another bot reply taking into consideration that it was already replied to
    if (lastReplyMsg != "")
    {
        if (lastReplyMsg == msg)
        {
            ostringstream out;
            out << "DEBUG ChatReplyDo msg has already been replied to, skipping";
            //bot->Say(out.str(), LANG_UNIVERSAL);

            return;
        }
    }




    // DEBUG
    ostringstream out;
    out << "DEBUG ChatReplyDo triggered, trying respond to following string:";
    // bot->Say(out.str(), LANG_UNIVERSAL);

    out << msg;
    //bot->Say(out.str(), LANG_UNIVERSAL);


    // filter by name or IDs...
    //if (name == "Jehuty" || "Ariderion")
    if (group)
    {
        if ((masterID > 4500 && masterID < 4600) && (botID > 4500 && botID < 4600))
        {

            // GPT Experiment Start
            //try {
            //    std::string response = sendToGPT(msg);

            //    ostringstream out;
            //    out << response;
            //    bot->Say(out.str(), LANG_UNIVERSAL);
            //}
            //catch (const std::runtime_error& e) {
            //    std::cerr << "Error: " << e.what() << std::endl;
            //}

            uint32 botLevel = bot->GetLevel();
            string botLevelString = std::to_string(botLevel);
            uint8 botClass = bot->getClass();
            string botClassString = "";
            uint8 botRace = bot->getRace();
            string botRaceString = "";
            uint8 botGender = bot->getGender();
            string botGenderString = "";

            uint32 botZone = bot->GetZoneId();
            string botZoneString = std::to_string(botZone);

            switch (botClass) {
            case CLASS_WARRIOR:
                botClassString = "Warrior";
                break;
            case CLASS_PALADIN:
                botClassString = "Paladin";
                break;
            case CLASS_HUNTER:
                botClassString = "Hunter";
                break;
            case CLASS_ROGUE:
                botClassString = "Rogue";
                break;
            case CLASS_PRIEST:
                botClassString = "Priest";
                break;
            case CLASS_SHAMAN:
                botClassString = "Shaman";
                break;
            case CLASS_MAGE:
                botClassString = "Mage";
                break;
            case CLASS_WARLOCK:
                botClassString = "Warlock";
                break;
            case CLASS_DRUID:
                botClassString = "Druid";
                break;
            default:
                botClassString = "Unknown Class";
            }

            switch (botRace) {
            case RACE_HUMAN:
                botRaceString = "Human";
                break;
            case RACE_ORC:
                botRaceString = "Orc";
                break;
            case RACE_DWARF:
                botRaceString = "Dwarf";
                break;
            case RACE_NIGHTELF:
                botRaceString = "Night Elf";
                break;
            case RACE_UNDEAD:
                botRaceString = "Undead";
                break;
            case RACE_TAUREN:
                botRaceString = "Tauren";
                break;
            case RACE_GNOME:
                botRaceString = "Gnome";
                break;
            case RACE_TROLL:
                botRaceString = "Troll";
                break;
            default:
                botRaceString = "Unknown Race";
            }


            if (botGender == GENDER_MALE)
                botGenderString = "male";
            else botGenderString = "female";

            std::string prevDialogueContext;
            auto prevDialogueValue = bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<std::string>("prev dialogue", "GPT");

            // Check if the value exists
            if (prevDialogueValue) {
                // If it exists, get the current value
                prevDialogueContext = prevDialogueValue->Get();
            }
            else {
                // If not, initialize it to an empty string
                prevDialogueContext = "";
            }

            try {
                // Concatenate the player's name with the system message
                std::string systemMessage = "You are " + botName + ", a level " + botLevelString + " " + botRaceString + " " + botClassString + " in ZoneID " + botZoneString + " Vanilla WoW. Respond to the player's chat message in 1 or 2 lines max. The player's name is " + name + ". Stay in character. The previous dialogue was: " + prevDialogueContext;

                if (strstr(bot->GetName(), "Yueh") != nullptr)
                    systemMessage = "You are Yueh, a lvl60 human male Holy Priest in Vanilla WoW. You are calm and calculated, well articulated and to the point. Respond to the player. 1-2 lines max. Others may be adressed as well. Location: " + botZoneString + ", Player name: " + name;

                if (strstr(bot->GetName(), "Paul") != nullptr)
                    systemMessage = "You are Paul, a lvl60 human holy Paladin in Vanilla WoW. You are a philosopher, favoring zensunni teachings from Frank Herbert's Dune to guide your thoughts. Respond to the player. 1-2 lines max. Others may be adressed as well. Location: " + botZoneString + ", Player name: " + name;

                if (strstr(bot->GetName(), "Sarrica") != nullptr)
                    systemMessage = "You are Sarrica, a reticent lvl60 night elf male DPS Warrior in Vanilla WoW, as Sentinel of Darnassus you are a calm but deadly warrior, who will do what is necessary to protect his people.. 1-2 lines max. Respond to the player's chat message. Others may be adressed as well. Location: " + botZoneString + ", Player name: " + name;

                // Set up the JSON payload with your messages
                nlohmann::json json_payload = {
                    {"model", "gpt-4-1106-preview"},
                    {"messages", {
                        {
                            {"role", "system"},
                            {"content", systemMessage}
                        },
                        {
                            {"role", "user"},
                            {"content", msg}
                        }
                    }},
                    {"temperature", 1},
                    {"max_tokens", 256},
                    {"top_p", 1},
                    {"frequency_penalty", 0},
                    {"presence_penalty", 0}
                };



                char* env_api_key = std::getenv("WOW_API_KEY");
                std::string api_key(env_api_key);

                // Convert JSON payload to string
                std::string payload_string = json_payload.dump();

                // Initialize CURL
                CURL* curl = curl_easy_init();
                if (curl) {
                    // Set the URL and other HTTP request properties
                    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
                    curl_easy_setopt(curl, CURLOPT_POST, 1L);
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)payload_string.size());
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_string.c_str());

                    struct curl_slist* headers = NULL;
                    headers = curl_slist_append(headers, "Content-Type: application/json");
                    std::string auth_header = "Authorization: Bearer " + api_key;
                    headers = curl_slist_append(headers, auth_header.c_str());
                    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

                    // Set up to capture the response
                    std::string response_string;
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

                    // Perform the request
                    CURLcode res = curl_easy_perform(curl);

                    // Check for errors
                    if (res != CURLE_OK)
                        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;

                    // Cleanup
                    curl_easy_cleanup(curl);
                    curl_slist_free_all(headers);

                    // Parse the response into JSON
                    auto response_json = nlohmann::json::parse(response_string);

                    // Extract the text from the response
                    std::string response_text = response_json["choices"][0]["message"]["content"];

                    // Output the response text to console
                    std::cout << "Response: " << response_text << std::endl;

                    //ostringstream out; out << response_text;
                    //bot->Say(out.str(), LANG_UNIVERSAL);

                    if (!response_text.empty())
                    {
                        const char* c = response_text.c_str();
                        if (strlen(c) > 255)
                            return;

                        if (chanName == "World")
                        {
                            return;
                        }
                        else
                        {
                            if (type == CHAT_MSG_PARTY)
                            {
                                WorldPacket data;
                                ChatHandler::BuildChatPacket(data, bot->GetGroup()->IsRaidGroup() ? CHAT_MSG_RAID : CHAT_MSG_PARTY, response_text.c_str(), LANG_UNIVERSAL, CHAT_TAG_NONE, bot->GetObjectGuid(), bot->GetName());
                                bot->GetGroup()->BroadcastPacket(data, true);

                            }
                            if (type == CHAT_MSG_WHISPER)
                            {
                                ObjectGuid receiver = sObjectMgr.GetPlayerGuidByName(name.c_str());
                                Player* rPlayer = sObjectMgr.GetPlayer(receiver);
                                if (rPlayer)
                                {
                                    if (bot->GetTeam() == ALLIANCE)
                                    {
                                        bot->Whisper(c, LANG_COMMON, receiver);
                                    }
                                    else
                                    {
                                        bot->Whisper(c, LANG_ORCISH, receiver);
                                    }
                                }
                            }

                            if (type == CHAT_MSG_SAY)
                            {
                                if (bot->GetTeam() == ALLIANCE)
                                    bot->Say(response_text, LANG_COMMON);
                                else
                                    bot->Say(response_text, LANG_ORCISH);
                            }

                            if (type == CHAT_MSG_YELL)
                            {
                                if (bot->GetTeam() == ALLIANCE)
                                    bot->Yell(response_text, LANG_COMMON);
                                else
                                    bot->Yell(response_text, LANG_ORCISH);
                            }

                            if (type == CHAT_MSG_GUILD)
                            {
                                if (!bot->GetGuildId())
                                    return;

                                if (Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId()))
                                    guild->BroadcastToGuild(bot->GetSession(), response_text, LANG_UNIVERSAL);
                            }
                        }

                        std::string appendDialogueContext = name + ": " + msg + " - " + botName + ": " + response_text;
                        const size_t maxContextLength = 512;

                        // Append the new line to the existing dialogue
                        prevDialogueContext += " | " + appendDialogueContext; // Using '|' as a separator, you can choose your own

                        if (prevDialogueContext.length() > maxContextLength) {
                            // Keep only the last 'maxLength' characters
                            prevDialogueContext = prevDialogueContext.substr(prevDialogueContext.length() - maxContextLength);
                        }
                        bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<std::string>("prev dialogue", "GPT")->Set(prevDialogueContext);

                        bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 15));
                        return;
                    }
                }
            }
            catch (std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }

        }
    }

    
    // GPT Experiment End


    // Chat Logic
    int32 verb_pos = -1;
    int32 verb_type = -1;
    int32 is_quest = 0;
    bool found = false;
    std::stringstream text(msg);
    std::string segment;
    std::vector<std::string> word;
    while (std::getline(text, segment, ' '))
    {
        word.push_back(segment);

        // each segment is a word in the line that triggered the bot
        //ostringstream out; out << segment;
        //bot->Say(out.str(), LANG_UNIVERSAL);
    }

    for (uint32 i = 0; i < 15; i++)
    {
        if (word.size() < i)
            word.push_back("");
    }

    if (msg.find("?") != std::string::npos)
        is_quest = 1;
    if (word[0].find("what") != std::string::npos)
        is_quest = 2;
    else if (word[0].find("who") != std::string::npos)
        is_quest = 3;
    else if (word[0] == "when")
        is_quest = 4;
    else if (word[0] == "where")
        is_quest = 5;
    else if (word[0] == "why")
        is_quest = 6;

    // Responds
    for (uint32 i = 0; i < 8; i++)
    {
        // blame gm with chat tag
        if (Player* plr = sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid1)))
        {
            if (plr->isGMChat())
            {
                replyType = REPLY_ADMIN_ABUSE;
                found = true;
                break;
            }
        }

        if (word[i] == "hi" || word[i] == "hey" || word[i] == "hello" || word[i] == "wazzup")
        {
            replyType = REPLY_HELLO;
            found = true;
            break;
        }

        if (verb_type < 4)
        {
            if (word[i] == "am" || word[i] == "are" || word[i] == "is")
            {
                verb_pos = i;
                verb_type = 2; // present
            }
            else if (word[i] == "will")
            {
                verb_pos = i;
                verb_type = 3; // future
            }
            else if (word[i] == "was" || word[i] == "were")
            {
                verb_pos = i;
                verb_type = 1; // past
            }
            else if (word[i] == "shut" || word[i] == "noob")
            {
                if (msg.find(bot->GetName()) == std::string::npos)
                {
                    continue; // not react
                    uint32 rnd = urand(0, 2);
                    std::string msg = "";
                    if (rnd == 0)
                        msg = "sorry %s, ill shut up now";
                    if (rnd == 1)
                        msg = "ok ok %s";
                    if (rnd == 2)
                        msg = "fine, i wont talk to you anymore %s";

                    msg = std::regex_replace(msg, std::regex("%s"), name);
                    respondsText = msg;
                    found = true;
                    break;
                }
                else
                {
                    replyType = REPLY_GRUDGE;
                    found = true;
                    break;
                }
            }
        }
    }
    if (verb_type < 4 && is_quest && !found)
    {
        switch (is_quest)
        {
        case 2:
        {
            uint32 rnd = urand(0, 3);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "i dont know what";
                break;
            case 1:
                msg = "i dont know %s";
                break;
            case 2:
                msg = "who cares";
                break;
            case 3:
                msg = "afraid that was before i was around or paying attention";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 3:
        {
            uint32 rnd = urand(0, 4);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "nobody";
                break;
            case 1:
                msg = "we all do";
                break;
            case 2:
                msg = "perhaps its you, %s";
                break;
            case 3:
                msg = "dunno %s";
                break;
            case 4:
                msg = "is it me?";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 4:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "soon perhaps %s";
                break;
            case 1:
                msg = "probably later";
                break;
            case 2:
                msg = "never";
                break;
            case 3:
                msg = "what do i look like, a psychic?";
                break;
            case 4:
                msg = "a few minutes, maybe an hour ... years?";
                break;
            case 5:
                msg = "when? good question %s";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 5:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "really want me to answer that?";
                break;
            case 1:
                msg = "on the map?";
                break;
            case 2:
                msg = "who cares";
                break;
            case 3:
                msg = "afk?";
                break;
            case 4:
                msg = "none of your buisiness where";
                break;
            case 5:
                msg = "yeah, where?";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 6:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "dunno %s";
                break;
            case 1:
                msg = "why? just because %s";
                break;
            case 2:
                msg = "why is the sky blue?";
                break;
            case 3:
                msg = "dont ask me %s, im just a bot";
                break;
            case 4:
                msg = "your asking the wrong person";
                break;
            case 5:
                msg = "who knows?";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        default:
        {
            switch (verb_type)
            {
            case 1:
            {
                uint32 rnd = urand(0, 3);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "its true, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 4];
                    break;
                case 1:
                    msg = "ya %s but thats in the past";
                    break;
                case 2:
                    msg = "nah, but " + word[verb_pos + 1] + " will " + word[verb_pos + 3] + " again though %s";
                    break;
                case 3:
                    msg = "afraid that was before i was around or paying attention";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 2:
            {
                uint32 rnd = urand(0, 6);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "its true, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                    break;
                case 1:
                    msg = "ya %s thats true";
                    break;
                case 2:
                    msg = "maybe " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                    break;
                case 3:
                    msg = "dunno %s";
                    break;
                case 4:
                    msg = "i dont think so %s";
                    break;
                case 5:
                    msg = "yes";
                    break;
                case 6:
                    msg = "no";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 3:
            {
                uint32 rnd = urand(0, 8);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "dunno %s";
                    break;
                case 1:
                    msg = "beats me %s";
                    break;
                case 2:
                    msg = "how should i know %s";
                    break;
                case 3:
                    msg = "dont ask me %s, im just a bot";
                    break;
                case 4:
                    msg = "your asking the wrong person";
                    break;
                case 5:
                    msg = "what do i look like, a psychic?";
                    break;
                case 6:
                    msg = "sure %s";
                    break;
                case 7:
                    msg = "i dont think so %s";
                    break;
                case 8:
                    msg = "maybe";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            }
        }
        }
    }
    else if (!found)
    {
        switch (verb_type)
        {
        case 1:
        {
            uint32 rnd = urand(0, 2);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "yeah %s, the key word being " + word[verb_pos] + " " + word[verb_pos + 1];
                break;
            case 1:
                msg = "ya %s but thats in the past";
                break;
            case 2:
                msg = word[verb_pos - 1] + " will " + word[verb_pos + 1] + " again though %s";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 2:
        {
            uint32 rnd = urand(0, 2);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "%s, what do you mean " + word[verb_pos + 1] + "?";
                break;
            case 1:
                msg = "%s, what is a " + word[verb_pos + 1] + "?";
                break;
            case 2:
                msg = "yeah i know " + word[verb_pos - 1] + " is a " + word[verb_pos + 1];
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 3:
        {
            uint32 rnd = urand(0, 1);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "are you sure thats going to happen %s?";
                break;
            case 1:
                msg = "%s, what will happen %s?";
                break;
            case 2:
                msg = "are you saying " + word[verb_pos - 1] + " will " + word[verb_pos + 1] + " " + word[verb_pos + 2] + " %s?";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        }
    }

    if (!found)
    {
        // Name Responds
        if (msg.find(bot->GetName()) != std::string::npos)
        {
            replyType = REPLY_NAME;
            found = true;
        }
        else // Does not understand
        {
            replyType = REPLY_NOT_UNDERSTAND;
            found = true;
        }
    }

    // send responds
        // 
    if (found)
    {
        // load text if needed
        if (respondsText.empty())
        {
            respondsText = BOT_TEXT2(replyType, name);
        }
        const char* c = respondsText.c_str();
        if (strlen(c) > 255)
            return;

        if (chanName == "World")
        {
            if (ChannelMgr* cMgr = channelMgr(bot->GetTeam()))
            {
                std::string worldChan = "World";
#ifndef MANGOSBOT_ZERO
                if (Channel* chn = cMgr->GetJoinChannel(worldChan.c_str(), 0))
#else
                if (Channel* chn = cMgr->GetJoinChannel(worldChan.c_str()))
#endif
                {
                    if (bot->GetTeam() == ALLIANCE)
                    {
                        chn->Say(bot, c, LANG_COMMON);
                    }
                    else
                    {
                        chn->Say(bot, c, LANG_ORCISH);
                    }
                }
            }
        }
        else
        {
            if (type == CHAT_MSG_WHISPER)
            {
                ObjectGuid receiver = sObjectMgr.GetPlayerGuidByName(name.c_str());
                Player* rPlayer = sObjectMgr.GetPlayer(receiver);
                if (rPlayer)
                {
                    if (bot->GetTeam() == ALLIANCE)
                    {
                        bot->Whisper(c, LANG_COMMON, receiver);
                    }
                    else
                    {
                        bot->Whisper(c, LANG_ORCISH, receiver);
                    }
                }
            }

            if (type == CHAT_MSG_SAY)
            {
                if (bot->GetTeam() == ALLIANCE)
                    bot->Say(respondsText, LANG_COMMON);
                else
                    bot->Say(respondsText, LANG_ORCISH);
            }

            if (type == CHAT_MSG_YELL)
            {
                if (bot->GetTeam() == ALLIANCE)
                    bot->Yell(respondsText, LANG_COMMON);
                else
                    bot->Yell(respondsText, LANG_ORCISH);
            }

            if (type == CHAT_MSG_GUILD)
            {
                if (!bot->GetGuildId())
                    return;

                if (Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId()))
                    guild->BroadcastToGuild(bot->GetSession(), respondsText, LANG_UNIVERSAL);
            }
        }
        bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 25));
    }
}
