#include "../botpch.h"
#include "playerbot.h"
#include "ChatFilter.h"
#include "strategy/values/RtiTargetValue.h"
#include "strategy/values/ItemUsageValue.h"
#include "ChatHelper.h"

using namespace ai;
using namespace std;

string ChatFilter::Filter(string message, string filter)
{
    if (message.find("@") == string::npos)
        return message;

    if(filter.empty())
        return message.substr(message.find(" ") + 1);

    return message.substr(message.find(filter) + filter.size() + 1);
}


class StrategyChatFilter : public ChatFilter
{
public:
    StrategyChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "strategy";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@nc=rpg"] = "All bots that have rpg strategy enabled in noncombat state.";
        retMap["@nonc=travel"] = "All bots that do not have travel strategy enabled in noncombat state.";
        retMap["@co=melee"] = "All bots that have melee strategy enabled in combat state.";
        retMap["@react=<>"] = "All bots that have <> strategy enabled in reaction state.";
        retMap["@dead=<>"] = "All bots that have <> strategy enabled in dead state.";
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots based on their strategies.";
    }
#endif

    virtual string Filter(string message) override
    {
        Player* bot = ai->GetBot();

        if (message.find("@nc=") == 0)
        {
            string strat = message.substr(message.find("=")+1, message.find(" ") - (message.find("=")+1));

            if (!strat.empty())
            {             
                if (ai->HasStrategy(strat,BotState::BOT_STATE_NON_COMBAT))
                    return ChatFilter::Filter(message);
            }
            return message;            
        }
        if (message.find("@nonc=") == 0)
        {
            string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (!ai->HasStrategy(strat, BotState::BOT_STATE_NON_COMBAT))
                    return ChatFilter::Filter(message);
            }

            return message;
        }
        if (message.find("@co=") == 0)
        {
            string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (ai->HasStrategy(strat, BotState::BOT_STATE_COMBAT))
                    return ChatFilter::Filter(message);
            }

            return message;
        }
        if (message.find("@noco=") == 0)
        {
            string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (!ai->HasStrategy(strat, BotState::BOT_STATE_COMBAT))
                    return ChatFilter::Filter(message);
            }

            return message;
        }
        if (message.find("@react=") == 0)
        {
            string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (ai->HasStrategy(strat, BotState::BOT_STATE_REACTION))
                    return ChatFilter::Filter(message);
            }

            return message;
        }
        if (message.find("@noreact=") == 0)
        {
            string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (!ai->HasStrategy(strat, BotState::BOT_STATE_REACTION))
                    return ChatFilter::Filter(message);
            }

            return message;
        }
        if (message.find("@dead=") == 0)
        {
            string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (ai->HasStrategy(strat, BotState::BOT_STATE_DEAD))
                    return ChatFilter::Filter(message);
            }

            return message;
        }
        if (message.find("@nodead=") == 0)
        {
            string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (!ai->HasStrategy(strat, BotState::BOT_STATE_DEAD))
                    return ChatFilter::Filter(message);
            }

            return message;
        }

        return message;
    }
};

class RoleChatFilter : public ChatFilter
{
public:
    RoleChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "role";
    }
    virtual unordered_map<string,string> GetFilterExamples() 
    {
        unordered_map<string, string> retMap;
        retMap["@tank"] = "All bots that have a tank spec.";
        retMap["@dps"] = "All bots that do not have a tank or healing spec.";
        retMap["@heal"] = "All bots that have a healing spec.";
        retMap["@notank"] = "All bots that do not have a tank spec.";
        retMap["@nodps"] = "All bots that have a tank or healing spec.";
        retMap["@noheal"] = "All bots that do not have a healing spec.";
        retMap["@ranged"] = "All bots that use ranged attacks.";
        retMap["@melee"] = "All bots that use melee attacks.";
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots with a specific role or weapon range.";
    }
#endif

    virtual string Filter(string message) override
    {
        Player* bot = ai->GetBot();
        const bool inGroup = bot->GetGroup() != nullptr;

        bool tank = message.find("@tank") == 0;
        if (tank && !ai->IsTank(bot, inGroup))
            return "";

        bool dps = message.find("@dps") == 0;
        if (dps && (ai->IsTank(bot, inGroup) || ai->IsHeal(bot, inGroup)))
            return "";

        bool heal = message.find("@heal") == 0;
        if (heal && !ai->IsHeal(bot, inGroup))
            return "";

        bool notank = message.find("@notank") == 0;
        if (notank && ai->IsTank(bot, inGroup))
            return "";

        bool nodps = message.find("@nodps") == 0;
        if (nodps && !ai->IsTank(bot, inGroup) && !ai->IsHeal(bot, inGroup))
            return "";

        bool noheal = message.find("@noheal") == 0;
        if (noheal && ai->IsHeal(bot, inGroup))
            return "";

        bool ranged = message.find("@ranged") == 0;
        if (ranged && !ai->IsRanged(bot, inGroup))
            return "";

        bool melee = message.find("@melee") == 0;
        if (melee && ai->IsRanged(bot, inGroup))
            return "";

        if (tank || dps || heal || ranged || melee || noheal || nodps || notank)
            return ChatFilter::Filter(message);

        return message;
    }
};

class LevelChatFilter : public ChatFilter
{
public:
    LevelChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "level";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@60"] = "All bots that are level 60.";
        retMap["@10-20"] = "All bots ranging from level 10 to 20.";
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots based on level.";
    }
#endif

    virtual string Filter(string message) override
    {
        Player* bot = ai->GetBot();

        if (message[0] != '@')
            return message;

        if (message.find("-") != string::npos)
        {
            uint32 fromLevel = atoi(message.substr(message.find("@") + 1, message.find("-")).c_str());
            uint32 toLevel = atoi(message.substr(message.find("-") + 1, message.find(" ")).c_str());

            if (bot->GetLevel() >= fromLevel && bot->GetLevel() <= toLevel)
                return ChatFilter::Filter(message);

            return message;
        }

        uint32 level = atoi(message.substr(message.find("@") + 1, message.find(" ")).c_str());
        if (bot->GetLevel() == level)
            return ChatFilter::Filter(message);

        return message;
    }
};

class GearChatFilter : public ChatFilter
{
public:
    GearChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "gear";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@tier1"] = "All bots that have an avarage item level comparable to tier1";
        retMap["@tier2-3"] = "All bots an avarage item level comparable to tier2 or tier3.";
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots based on gear level.";
    }
#endif

    virtual string Filter(string message) override
    {
        Player* bot = ai->GetBot();

        if (message.find("@tier") != 0)
            return message;

        uint32 fromLevel;
        uint32 toLevel;
        uint32 botTier = 0;
        uint32 gs = ai->GetEquipGearScore(bot, false, false);

        if (message.find("-") != string::npos)
        {
            fromLevel = atoi(message.substr(message.find("@tier") + 5, message.find("-")).c_str());
            toLevel = atoi(message.substr(message.find("-") + 1, message.find(" ")).c_str());
        }
        else
        {
            fromLevel = atoi(message.substr(message.find("@tier") + 5, message.find(" ")).c_str());
            toLevel = atoi(message.substr(message.find("@tier") + 5, message.find(" ")).c_str());
        }

        if (gs > 60 && gs < 70)
            botTier = 1;
        else if (gs >= 71 && gs <= 76)
            botTier = 2;
        else if (gs >= 77 && gs <= 99)
            botTier = 3;
        else if (gs >= 120 && gs <= 132)
            botTier = 4;
        else if (gs >= 133 && gs <= 145)
            botTier = 5;
        else if (gs >= 146 && gs <= 154)
            botTier = 6;
        else if (gs >= 200 && gs <= 213)
            botTier = 7;
        else if (gs >= 225 && gs <= 232)
            botTier = 8;
        else if (gs >= 232 && gs <= 245)
            botTier = 9;
        else if (gs >= 251 && gs <= 277)
            botTier = 10;

        if (botTier >= fromLevel && botTier <= toLevel)
            return ChatFilter::Filter(message);

        return message;

    }
};

class CombatTypeChatFilter : public ChatFilter
{
public:
    CombatTypeChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "combat";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@ranged"] = "All bots that use ranged attacks.";
        retMap["@melee"] = "All bots that use melee attacks.";
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots with a specific weapon range.";
    }
#endif

    virtual string Filter(string message) override
    {
        Player* bot = ai->GetBot();

        bool melee = message.find("@melee") == 0;
        bool ranged = message.find("@ranged") == 0;

        if (!melee && !ranged)
            return message;

        switch (bot->getClass())
        {
            case CLASS_WARRIOR:
            case CLASS_PALADIN:
            case CLASS_ROGUE:
            /*case CLASS_DEATH_KNIGHT:
                if (ranged)
                    return "";
                break;*/

            case CLASS_HUNTER:
            case CLASS_PRIEST:
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                if (melee)
                    return "";
                break;

            case CLASS_DRUID:
                if (ranged && ai->IsTank(bot,false))
                    return "";
                if (melee && !ai->IsTank(bot, false))
                    return "";
                break;

            case CLASS_SHAMAN:
                if (melee && ai->IsHeal(bot, false))
                    return "";
                if (ranged && !ai->IsHeal(bot, false))
                    return "";
                break;
        }

        return ChatFilter::Filter(message);
    }
};

class RtiChatFilter : public ChatFilter
{
public:
#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "rti";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@star"] = "All bots that are marked with or are targeing something marked with star.";
        retMap["@circle"] = "All bots that are marked with or are targeing something marked with star.";
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots that are marked with or are targeting sonething marked with a raid target icon.";
    }
#endif

    RtiChatFilter(PlayerbotAI* ai) : ChatFilter(ai)
    {
        rtis.push_back("@star");
        rtis.push_back("@circle");
        rtis.push_back("@diamond");
        rtis.push_back("@triangle");
        rtis.push_back("@moon");
        rtis.push_back("@square");
        rtis.push_back("@cross");
        rtis.push_back("@skull");
    }

    virtual string Filter(string message) override
    {
        Player* bot = ai->GetBot();
        Group *group = bot->GetGroup();
        if(!group)
            return message;

        bool found = false;
        bool isRti = false;
        for (list<string>::iterator i = rtis.begin(); i != rtis.end(); i++)
        {
            string rti = *i;

            bool isRti = message.find(rti) == 0;
            if (!isRti)
                continue;

            ObjectGuid rtiTarget = group->GetTargetIcon(RtiTargetValue::GetRtiIndex(rti.substr(1)));
            if (bot->GetObjectGuid() == rtiTarget)
                return ChatFilter::Filter(message);

            Unit* target = *ai->GetAiObjectContext()->GetValue<Unit*>("current target");
            if (!target)
                return "";

            if (target->GetObjectGuid() != rtiTarget)
                return "";

            found |= isRti;
            if (found)
                break;
        }

        if (found)
            return ChatFilter::Filter(message);

        return message;
    }

private:
    list<string> rtis;
};

class ClassChatFilter : public ChatFilter
{
public:
    ClassChatFilter(PlayerbotAI* ai) : ChatFilter(ai)
    {
#ifdef MANGOSBOT_TWO
        classNames["@deathknight"] = CLASS_DEATH_KNIGHT;
#endif
        classNames["@druid"] = CLASS_DRUID;
        classNames["@hunter"] = CLASS_HUNTER;
        classNames["@mage"] = CLASS_MAGE;
        classNames["@paladin"] = CLASS_PALADIN;
        classNames["@priest"] = CLASS_PRIEST;
        classNames["@rogue"] = CLASS_ROGUE;
        classNames["@shaman"] = CLASS_SHAMAN;
        classNames["@warlock"] = CLASS_WARLOCK;
        classNames["@warrior"] = CLASS_WARRIOR;
    }

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "class";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@rogue"] = "All rogue bots.";
        retMap["@warlock"] = "All warlock bots.";
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots have a certain class.";
    }
#endif

    virtual string Filter(string message) override
    {
        Player* bot = ai->GetBot();

        bool found = false;
        bool isClass = false;
        for (map<string, uint8>::iterator i = classNames.begin(); i != classNames.end(); i++)
        {
            bool isClass = message.find(i->first) == 0;
            if (isClass && bot->getClass() != i->second)
                return "";

            found |= isClass;
            if (found)
                break;
        }

        if (found)
            return ChatFilter::Filter(message);

        return message;
    }

private:
    map<string, uint8> classNames;
};

class GroupChatFilter : public ChatFilter
{
public:
    GroupChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "group";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@group"] = "All bots in a group.";
        retMap["@group2"] = "All bots in group 2.";
        retMap["@group4-6"] = "All bots in group 4 to 6.";
        retMap["@nogroup"] = "All bots that are not grouped.";
        retMap["@leader"] = "All bots that are leader of their group.";
        retMap["@raid"] = "All bots that are in a raid group.";
        retMap["@noraid"] = "All bots that are not in a raid group.";
        retMap["@rleader"] = "All bots that are leader of a raid group.";
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots based on their group.";
    }
#endif

    virtual string Filter(string message) override
    {
        Player* bot = ai->GetBot();

        if (message.find("@group") == 0)
        {
            string pnum = message.substr(6, message.find(" ")-6);
            int from = atoi(pnum.c_str());
            int to = from;
            if (pnum.find("-") != string::npos)
            {
                from = atoi(pnum.substr(pnum.find("@") + 1, pnum.find("-")).c_str());
                to = atoi(pnum.substr(pnum.find("-") + 1, pnum.find(" ")).c_str());
            }

            if (!bot->GetGroup())
                return message;

            int sg = (int)bot->GetSubGroup() + 1;
            if (sg >= from && sg <= to)
                return ChatFilter::Filter(message);
        }
        if (message.find("@nogroup") == 0)
        {
            if (bot->GetGroup())
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@leader") == 0)
        {
            if (!bot->GetGroup())
                return message;

            if(bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
                return ChatFilter::Filter(message);
        }
        if (message.find("@raid") == 0)
        {
            if (!bot->GetGroup() || !bot->GetGroup()->IsRaidGroup())
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@noraid") == 0)
        {
            if (bot->GetGroup() && bot->GetGroup()->IsRaidGroup())
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@rleader") == 0)
        {
            if (!bot->GetGroup())
                return message;

            if (!bot->GetGroup()->IsRaidGroup())
                return message;

            if (bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
                return ChatFilter::Filter(message);
        }

        return message;
    }
};

class GuildChatFilter : public ChatFilter
{
public:
    GuildChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "guild";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@guild"] = "All bots in a guild.";
        retMap["@guild=raiders"] = "All bots in guild raiders.";
        retMap["@noguild"] = "All bots not in a guild.";
        retMap["@gleader"] = "All bots that are a guild leader.";
        retMap["@rank=Initiate"] = "All bots that have rank Initiate in their guild.";
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots based on their guild.";
    }
#endif

    virtual string Filter(string message) override
    {
        Player* bot = ai->GetBot();

        if (message.find("@guild=") == 0)
        {
            if (!bot->GetGuildId())
                return message;

            string pguild = message.substr(7, message.find(" ")-7);

            if (!pguild.empty())
            {
                Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
                string guildName = guild->GetName();

                if (pguild.find(guildName) != 0)
                    return message;
            }

            return ChatFilter::Filter(message);
        }
        if (message.find("@guild") == 0)
        {
            if (!bot->GetGuildId())
                return message;

            string pguild = message.substr(6, message.find(" "));
            
            if (!pguild.empty())
            {
                Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
                string guildName = guild->GetName();

                if (pguild.find(guildName) == 0)
                    return message;
            }

            return ChatFilter::Filter(message);
        }
        if (message.find("@noguild") == 0)
        {
            if (bot->GetGuildId())
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@gleader") == 0)
        {
            if (!bot->GetGuildId())
                return message;

            Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
            if (guild->GetLeaderGuid() != bot->GetObjectGuid())
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@rank=") == 0)
        {
            if (!bot->GetGuildId())
                return message;

            string rank = message.substr(6, message.find(" ")-6);

            if (!rank.empty())
            {
                Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
                string rankName = guild->GetRankName(guild->GetRank(bot->GetObjectGuid()));

                if (rank.find(rankName) != 0)
                    return message;
            }

            return ChatFilter::Filter(message);
        }

        return message;
    }
};

class StateChatFilter : public ChatFilter
{
public:
    StateChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "state";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@needrepair"] = "All bots that have durability below 20%.";
        retMap["@outside"] = "All bots that are outside of an instance.";
        retMap["@inside"] = "All bots that are inside an instance.";
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots based on their state.";
    }
#endif

    virtual string Filter(string message) override
    {
        Player* bot = ai->GetBot();
        AiObjectContext* context = bot->GetPlayerbotAI()->GetAiObjectContext();

        if (message.find("@needrepair") == 0)
        {
            if (AI_VALUE(uint8, "durability") > 20)
                return message;
          
            return ChatFilter::Filter(message);
        }
        if (message.find("@outside") == 0)
        {
            if (!WorldPosition(bot).isOverworld())
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@inside") == 0)
        {
            if (WorldPosition(bot).isOverworld())
                return message;

            return ChatFilter::Filter(message);
        }        

        return message;
    }
};

class UsageChatFilter : public ChatFilter
{
public:
    UsageChatFilter(PlayerbotAI * ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "usage";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@use=[itemlink]"] = "All bots that have some use for this item.";
        retMap["@sell=[itemlink]"] = "All bots that will vendor or AH this item.";
        retMap["@need=[itemlink]"] = "All bots that will roll need on this item.";
        retMap["@greed=[itemlink]"] = "All bots that will roll greed on this item.";        
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots based on the use they have for a specific item.";
    }
#endif

    string FilterLink(string message)
    {
        if (message.find("@") == string::npos)
            return message;

        return message.substr(message.find("|r ") + 3);
    }

    virtual string Filter(string message) override
    {
        Player* bot = ai->GetBot();

        AiObjectContext* context = ai->GetAiObjectContext();

        if (message.find("@use=") == 0)
        {
            string item = message.substr(message.find("=") + 1, message.find("|r ") - (message.find("=") + 3));

            if (item.empty())
                return message;

            set<string> qualifiers = ChatHelper::parseItemQualifiers(item);

            if(qualifiers.empty())
                return message;

            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(*qualifiers.begin()).GetQualifier());

            if (usage != ItemUsage::ITEM_USAGE_NONE && usage != ItemUsage::ITEM_USAGE_AH && usage != ItemUsage::ITEM_USAGE_VENDOR)
            {
                return FilterLink(message);
            }

            return message;
        }
        if (message.find("@sell=") == 0)
        {
            string item = message.substr(message.find("=") + 1, message.find("|r ") - (message.find("=") + 3));

            if (item.empty())
                return message;

            set<string> qualifiers = ChatHelper::parseItemQualifiers(item);

            if (qualifiers.empty())
                return message;

            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(*qualifiers.begin()).GetQualifier());

            if (usage == ItemUsage::ITEM_USAGE_AH || usage == ItemUsage::ITEM_USAGE_VENDOR)
            {
                return FilterLink(message);
            }

            return message;
        }
        if (message.find("@need=") == 0)
        {
            string item = message.substr(message.find("=") + 1, message.find("|r ") - (message.find("=") + 3));

            if (item.empty())
                return message;

            set<string> qualifiers = ChatHelper::parseItemQualifiers(item);

            if (qualifiers.empty())
                return message;

            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(*qualifiers.begin()).GetQualifier());

            if (usage == ItemUsage::ITEM_USAGE_EQUIP || usage == ItemUsage::ITEM_USAGE_GUILD_TASK || usage == ItemUsage::ITEM_USAGE_BAD_EQUIP || usage == ItemUsage::ITEM_USAGE_FORCE_NEED)
            {
                return FilterLink(message);
            }

            return message;
        }
        if (message.find("@greed=") == 0)
        {
            string item = message.substr(message.find("=") + 1, message.find("|r ") - (message.find("=") + 3));

            if (item.empty())
                return message;

            set<string> qualifiers = ChatHelper::parseItemQualifiers(item);

            if (qualifiers.empty())
                return message;

            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(*qualifiers.begin()).GetQualifier());

            if (usage == ItemUsage::ITEM_USAGE_SKILL || usage == ItemUsage::ITEM_USAGE_USE || usage == ItemUsage::ITEM_USAGE_DISENCHANT || usage == ItemUsage::ITEM_USAGE_AH || usage == ItemUsage::ITEM_USAGE_VENDOR || usage == ItemUsage::ITEM_USAGE_FORCE_GREED)
            {
                return FilterLink(message);
            }

            return message;
        }
       
        return message;
    }
};

class TalentSpecChatFilter : public ChatFilter
{
public:
    TalentSpecChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "talent spec";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@frost"] = "All bots that have frost spec.";
        retMap["@holy"] = "All bots that have holy spec.";
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots based on their primary talent specialisation.";
    }
#endif

    virtual string Filter(string message) override
    {
        Player* bot = ai->GetBot();

        AiObjectContext* context = ai->GetAiObjectContext();
        string filter = "@" + ChatHelper::specName(bot);

        if (message.find(filter) == 0)
        {
            return ChatFilter::Filter(message);
        }
        
        return message;
    }
};

class LocationChatFilter : public ChatFilter
{
public:
    LocationChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "location";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@azeroth"] = "All bots in azeroth overworld.";
        retMap["@eastern kingdoms"] = "All bots in eastern kingdoms overworld.";
        retMap["@dun morogh"] = "All bots in the dun morogh zone.";
        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects bots based on map or zone name.";
    }
#endif

    virtual string Filter(string message) override
    {
        if (message.find("@") == 0)
        {

            Player* bot = ai->GetBot();

            AiObjectContext* context = ai->GetAiObjectContext();

            Map* map = bot->GetMap();

            if (map)
            {
                string name = map->GetMapName();
                string filter = name;

               std::transform(filter.begin(), filter.end(), filter.begin(),[](unsigned char c) { return std::tolower(c); });

                filter = "@" + filter;

                if (message.find(filter) == 0)
                {
                    return ChatFilter::Filter(message, filter);
                }
            }

            if (bot->GetTerrain())
            {
                string name = WorldPosition(bot).getAreaName(true, true);
                string filter = name;

                std::transform(filter.begin(), filter.end(), filter.begin(),[](unsigned char c) { return std::tolower(c); });

                filter = "@" + filter;

                if (message.find(filter) == 0)
                {
                    return ChatFilter::Filter(message, filter);
                }
            }
        }
        return message;
    }
};

class RandomChatFilter : public ChatFilter
{
public:
    RandomChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual string GetHelpName() {
        return "random";
    }
    virtual unordered_map<string, string> GetFilterExamples()
    {
        unordered_map<string, string> retMap;
        retMap["@random"] = "50% chance the bot responds.";
        retMap["@random=25"] = "25% chance the bot responds.";
        retMap["@fixedrandom"] = "50% chance the bot responds. But always the same bots.";
        retMap["@fixedrandom=25"] = "25% chance the bot responds. But always the same bots.";

        return retMap;
    }
    virtual string GetHelpDescription() {
        return "This filter selects random bots.";
    }
#endif

    virtual string Filter(string message) override
    {
        if (message.find("@random=") == 0)
        {
            string num = message.substr(message.find("=") + 1, message.find(" ") - message.find("=")-1);            
            if (urand(0, 100) < stoul(num))
                return ChatFilter::Filter(message);

            return message;
        }
        if (message.find("@random") == 0)
        {
            if (urand(0, 100) < 50)
                return ChatFilter::Filter(message);
        }
        if (message.find("@fixedrandom=") == 0)
        {
            string num = message.substr(message.find("=") + 1, message.find(" ") - message.find("=") - 1);
            if (ai->GetFixedBotNumer(BotTypeNumber::CHATFILTER_NUMBER) < stoul(num))
                return ChatFilter::Filter(message);

            return message;
        }
        if (message.find("@fixedrandom") == 0)
        {
            if (ai->GetFixedBotNumer(BotTypeNumber::CHATFILTER_NUMBER) < 50)
                return ChatFilter::Filter(message);
        }

        return message;
    }
};

CompositeChatFilter::CompositeChatFilter(PlayerbotAI* ai) : ChatFilter(ai)
{
    filters.push_back(new StrategyChatFilter(ai));
    filters.push_back(new RoleChatFilter(ai));
    filters.push_back(new ClassChatFilter(ai));
    filters.push_back(new RtiChatFilter(ai));
    filters.push_back(new CombatTypeChatFilter(ai));
    filters.push_back(new LevelChatFilter(ai));
    filters.push_back(new GroupChatFilter(ai));
    filters.push_back(new GuildChatFilter(ai));
    filters.push_back(new StateChatFilter(ai));
    filters.push_back(new UsageChatFilter(ai));
    filters.push_back(new TalentSpecChatFilter(ai));
    filters.push_back(new LocationChatFilter(ai));
    filters.push_back(new RandomChatFilter(ai));
    filters.push_back(new GearChatFilter(ai));
    
}

CompositeChatFilter::~CompositeChatFilter()
{
    for (list<ChatFilter*>::iterator i = filters.begin(); i != filters.end(); i++)
        delete (*i);
}

string CompositeChatFilter::Filter(string message)
{
    for (int j = 0; j < filters.size(); ++j)
    {
        for (list<ChatFilter*>::iterator i = filters.begin(); i != filters.end(); i++)
        {
            message = (*i)->Filter(message);
            if (message.empty())
                break;
        }
    }

    return message;
}

