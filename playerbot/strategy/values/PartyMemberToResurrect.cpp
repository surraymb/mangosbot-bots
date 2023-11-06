#include "botpch.h"
#include "../../playerbot.h"
#include "PartyMemberToResurrect.h"

#include "../../ServerFacade.h"
using namespace ai;

class IsTargetOfResurrectSpell : public SpellEntryPredicate
{
public:
    virtual bool Check(SpellEntry const* spell)
    {
        for (int i=0; i<3; i++)
        {
            if (spell->Effect[i] == SPELL_EFFECT_RESURRECT ||
                spell->Effect[i] == SPELL_EFFECT_RESURRECT_NEW ||
                spell->Effect[i] == SPELL_EFFECT_SELF_RESURRECT)
                return true;
        }
        return false;
    }

};

class FindDeadPlayer : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    FindDeadPlayer(PlayerbotAI* ai, PartyMemberValue* value) : PlayerbotAIAware(ai), value(value) {}

    virtual bool Check(Unit* unit)
    {
        Player* player = dynamic_cast<Player*>(unit);
        return player && !player->isRessurectRequested() && sServerFacade.GetDistance2d(ai->GetBot(), player) <= ai->GetRange("spell") && sServerFacade.GetDeathState(player) == CORPSE && !value->IsTargetOfSpellCast(player, predicate);
    }

private:
    PartyMemberValue* value;
    IsTargetOfResurrectSpell predicate;
};

Unit* PartyMemberToResurrect::Calculate()
{
    if (bot->InBattleGround())
    {
        return false; // CUSTOM attempt to disable bots trying to resurrection teammates in graveyards spending their mana on nothing. possibly disables player commanded resurrections too
    }

	FindDeadPlayer finder(ai, this);
    return FindPartyMember(finder);
}
