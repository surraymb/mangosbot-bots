#pragma once
#include "../Trigger.h"
#include "../../PlayerbotAIConfig.h"
#include "ServerFacade.h"
#include "SpellAuraDefines.h"

namespace ai
{
	class StatAvailable : public Trigger
	{
	public:
		StatAvailable(PlayerbotAI* ai, int amount, string name = "stat available") : Trigger(ai, name)
		{
			this->amount = amount;
		}

	protected:
		int amount;
	};

	class RageAvailable : public StatAvailable
    {
    public:
        RageAvailable(PlayerbotAI* ai, int amount) : StatAvailable(ai, amount, "rage available") {}
        virtual bool IsActive();
    };

    class LightRageAvailableTrigger : public RageAvailable
    {
    public:
        LightRageAvailableTrigger(PlayerbotAI* ai) : RageAvailable(ai, 20) {}
    };

    class MediumRageAvailableTrigger : public RageAvailable
    {
    public:
        MediumRageAvailableTrigger(PlayerbotAI* ai) : RageAvailable(ai, 40) {}
    };

    class HighRageAvailableTrigger : public RageAvailable
    {
    public:
        HighRageAvailableTrigger(PlayerbotAI* ai) : RageAvailable(ai, 60) {}
    };

	class EnergyAvailable : public StatAvailable
	{
	public:
		EnergyAvailable(PlayerbotAI* ai, int amount) : StatAvailable(ai, amount, "energy available") {}
		virtual bool IsActive();
	};

    class NoEnergyAvailableTrigger : public EnergyAvailable
    {
    public:
        NoEnergyAvailableTrigger(PlayerbotAI* ai) : EnergyAvailable(ai, 20) {}

        bool IsActive() override
        {
            return AI_VALUE2(uint8, "energy", "self target") < amount;
        }
    };

    class LightEnergyAvailableTrigger : public EnergyAvailable
    {
    public:
        LightEnergyAvailableTrigger(PlayerbotAI* ai) : EnergyAvailable(ai, 20) {}
    };

    class MediumEnergyAvailableTrigger : public EnergyAvailable
    {
    public:
        MediumEnergyAvailableTrigger(PlayerbotAI* ai) : EnergyAvailable(ai, 40) {}
    };

    class HighEnergyAvailableTrigger : public EnergyAvailable
    {
    public:
        HighEnergyAvailableTrigger(PlayerbotAI* ai) : EnergyAvailable(ai, 60) {}
    };

	class ComboPointsAvailableTrigger : public StatAvailable
	{
	public:
	    ComboPointsAvailableTrigger(PlayerbotAI* ai, uint8 amount = 5) : StatAvailable(ai, amount, "combo points available") {}
		virtual bool IsActive();
	};

	class LoseAggroTrigger : public Trigger 
    {
	public:
		LoseAggroTrigger(PlayerbotAI* ai) : Trigger(ai, "lose aggro") {}
		virtual bool IsActive();
	};

	class HasAggroTrigger : public Trigger 
    {
	public:
	    HasAggroTrigger(PlayerbotAI* ai) : Trigger(ai, "have aggro") {}
		virtual bool IsActive();
	};

	class SpellTrigger : public Trigger
	{
	public:
		SpellTrigger(PlayerbotAI* ai, string spell, int checkInterval = 1) : Trigger(ai, spell, checkInterval)
		{
			this->spell = spell;
		}

		virtual string GetTargetName() { return "current target"; }
		virtual string getName() { return spell; }
		virtual bool IsActive();

	protected:
		string spell;
	};

	class SpellCanBeCastTrigger : public SpellTrigger
	{
	public:
		SpellCanBeCastTrigger(PlayerbotAI* ai, string spell) : SpellTrigger(ai, spell) {}
		virtual bool IsActive();
	};

    class SpellNoCooldownTrigger : public SpellTrigger
    {
    public:
        SpellNoCooldownTrigger(PlayerbotAI* ai, string spell) : SpellTrigger(ai, spell) {}
        virtual bool IsActive();
    };

	// TODO: check other targets
    class InterruptSpellTrigger : public SpellTrigger
	{
    public:
        InterruptSpellTrigger(PlayerbotAI* ai, string spell) : SpellTrigger(ai, spell) {}
        virtual string GetTargetName() { return "current target"; }
        virtual bool IsActive();
    };

    class TargetOfAttacker : public Trigger
    {
    public:
        TargetOfAttacker(PlayerbotAI* ai, string name = "target of attacker", int checkInterval = 1) : Trigger(ai, name, checkInterval) {}
        virtual bool IsActive() override;

    protected:
        std::list<Unit*> GetAttackers();
    };

    class TargetOfAttackerInRange : public TargetOfAttacker
    {
    public:
        TargetOfAttackerInRange(PlayerbotAI* ai, string name, float distance, int checkInterval = 1) : TargetOfAttacker(ai, name, checkInterval), distance(distance) {}
        virtual bool IsActive() override;

    private:
        float distance;
    };

    class TargetOfAttackerClose : public TargetOfAttackerInRange
    {
    public:
        TargetOfAttackerClose(PlayerbotAI* ai) : TargetOfAttackerInRange(ai, "target of attacker close", sPlayerbotAIConfig.tooCloseDistance) {}
    };

    class TargetOfCastedAuraTypeTrigger : public Trigger
    {
    public:
        TargetOfCastedAuraTypeTrigger(PlayerbotAI* ai, string name, AuraType auraType, int checkInterval = 1) : Trigger(ai, name, checkInterval), auraType(auraType) {}
        virtual bool IsActive() override;

    private:
        AuraType auraType;
    };

    class TargetOfFearCastTrigger : public TargetOfCastedAuraTypeTrigger
    {
    public:
        TargetOfFearCastTrigger(PlayerbotAI* ai, string name = "target of fear cast") : TargetOfCastedAuraTypeTrigger(ai, name, AuraType::SPELL_AURA_MOD_FEAR) {}
    };

    class DeflectSpellTrigger : public SpellTrigger
    {
    public:
        DeflectSpellTrigger(PlayerbotAI* ai, string spell) : SpellTrigger(ai, spell) {}
        virtual bool IsActive();
    };

    class AttackerCountTrigger : public Trigger
    {
    public:
        AttackerCountTrigger(PlayerbotAI* ai, int amount, float distance = sPlayerbotAIConfig.sightDistance) : Trigger(ai, "attackers count", 2)
        {
            this->amount = amount;
            this->distance = distance;
        }

        virtual bool IsActive()
		{
            return AI_VALUE(uint8, "attackers count") >= amount;
        }

        virtual string getName() { return "attackers count"; }

    protected:
        int amount;
        float distance;
    };

    class HasAttackersTrigger : public AttackerCountTrigger
    {
    public:
        HasAttackersTrigger(PlayerbotAI* ai) : AttackerCountTrigger(ai, 1) {}
    };

    class MyAttackerCountTrigger : public AttackerCountTrigger
    {
    public:
        MyAttackerCountTrigger(PlayerbotAI* ai, int amount) : AttackerCountTrigger(ai, amount) {}
        virtual bool IsActive();
        virtual string getName() { return "my attacker count"; }
    };

    class MultipleAttackersTrigger : public MyAttackerCountTrigger
    {
    public:
        MultipleAttackersTrigger(PlayerbotAI* ai) : MyAttackerCountTrigger(ai, 2) {}
    };

    class HighThreatTrigger : public Trigger
    {
    public:
        HighThreatTrigger(PlayerbotAI* ai, string name = "high threat", int checkinterval = 1) : Trigger(ai, name, checkinterval) {}
        virtual bool IsActive();
    };

    class MediumThreatTrigger : public Trigger
    {
    public:
        MediumThreatTrigger(PlayerbotAI* ai, string name = "medium threat", int checkinterval = 1) : Trigger(ai, name, checkinterval) {}
        virtual bool IsActive();
    };

    class SomeThreatTrigger : public Trigger
    {
    public:
        SomeThreatTrigger(PlayerbotAI* ai, string name = "some threat", int checkinterval = 1) : Trigger(ai, name, checkinterval) {}
        virtual bool IsActive();
    };

    class NoThreatTrigger : public SomeThreatTrigger
    {
    public:
        NoThreatTrigger(PlayerbotAI* ai, string name = "some threat", int checkinterval = 1) : SomeThreatTrigger(ai, name, checkinterval) {}
        virtual bool IsActive();
    };

    class AoeTrigger : public AttackerCountTrigger
    {
    public:
        AoeTrigger(PlayerbotAI* ai, int amount = 3, float range = 40.0f) : AttackerCountTrigger(ai, amount), range(range) {}
        string getName() override { return "aoe"; }
        bool IsActive() override;

    private:
        float range;
    };

    class MeleeLightAoeTrigger : public AoeTrigger
    {
    public:
        MeleeLightAoeTrigger(PlayerbotAI* ai) : AoeTrigger(ai, 2, 5.0f) {}
    };

    class MeleeMediumAoeTrigger : public AoeTrigger
    {
    public:
        MeleeMediumAoeTrigger(PlayerbotAI* ai) : AoeTrigger(ai, 3, 5.0f) {}
    };

    class MeleeHighAoeTrigger : public AoeTrigger
    {
    public:
        MeleeHighAoeTrigger(PlayerbotAI* ai) : AoeTrigger(ai, 6, 5.0f) {}
    };

    class MeleeVeryHighAoeTrigger : public AoeTrigger
    {
    public:
        MeleeVeryHighAoeTrigger(PlayerbotAI* ai) : AoeTrigger(ai, 10, 5.0f) {}
    };

    class RangedLightAoeTrigger : public AoeTrigger
    {
    public:
        RangedLightAoeTrigger(PlayerbotAI* ai) : AoeTrigger(ai, 2) {}
    };

    class RangedMediumAoeTrigger : public AoeTrigger
    {
    public:
        RangedMediumAoeTrigger(PlayerbotAI* ai) : AoeTrigger(ai, 3) {}
    };

    class RangedHighAoeTrigger : public AoeTrigger
    {
    public:
        RangedHighAoeTrigger(PlayerbotAI* ai) : AoeTrigger(ai, 6) {}
    };

    class RangedVeryHighAoeTrigger : public AoeTrigger
    {
    public:
        RangedVeryHighAoeTrigger(PlayerbotAI* ai) : AoeTrigger(ai, 10) {}
    };

    class BuffTrigger : public SpellTrigger
    {
    public:
        BuffTrigger(PlayerbotAI* ai, string spell, int checkInterval = 1, bool checkIsOwner = false) : SpellTrigger(ai, spell, checkInterval) { this->checkIsOwner = checkIsOwner; }

    public:
		virtual string GetTargetName() { return "self target"; }
        virtual bool IsActive();

    protected:
        bool checkIsOwner;
    };

    class MyBuffTrigger : public BuffTrigger
    {
    public:
        MyBuffTrigger(PlayerbotAI* ai, string spell, int checkInterval = 1) : BuffTrigger(ai, spell, checkInterval) {}

    public:
        virtual bool IsActive();
    };

    class BuffOnPartyTrigger : public BuffTrigger
    {
    public:
        BuffOnPartyTrigger(PlayerbotAI* ai, string spell, int checkInterval = 2, bool ignoreTanks = false) : BuffTrigger(ai, spell, checkInterval), ignoreTanks(ignoreTanks) {}

    public:
		virtual Value<Unit*>* GetTargetValue();
		virtual string getName() { return spell + " on party"; }

    protected:
        bool ignoreTanks;
    };

    class GreaterBuffOnPartyTrigger : public BuffOnPartyTrigger
    {
    public:
        GreaterBuffOnPartyTrigger(PlayerbotAI* ai, string spell, string lowerSpell, int checkInterval = 2, bool ignoreTanks = false) : BuffOnPartyTrigger(ai, spell, checkInterval, ignoreTanks), lowerSpell(lowerSpell) {}
        virtual Value<Unit*>* GetTargetValue() override;
        virtual bool IsActive() override;

    private:
        std::string lowerSpell;
    };

    class BuffOnTankTrigger : public BuffTrigger
    {
    public:
        BuffOnTankTrigger(PlayerbotAI* ai, string spell, int checkInterval = 2) : BuffTrigger(ai, spell, checkInterval) {}

    public:
        virtual Value<Unit*>* GetTargetValue();
        virtual string getName() { return spell + " on tank"; }
    };

    class MyBuffOnPartyTrigger : public BuffOnPartyTrigger
    {
    public:
        MyBuffOnPartyTrigger(PlayerbotAI* ai, string spell, int checkInterval = 2, bool ignoreTanks = false) : BuffOnPartyTrigger(ai, spell, checkInterval, ignoreTanks) {}

    public:
        virtual Value<Unit*>* GetTargetValue();
        virtual string getName() { return spell + " on party"; }
    };

    class NoBuffAndComboPointsAvailableTrigger : public BuffTrigger
    {
    public:
        NoBuffAndComboPointsAvailableTrigger(PlayerbotAI* ai, string spell, uint8 comboPoints, bool checkIsOwner = true, int checkInterval = 1) : BuffTrigger(ai, spell, checkInterval, checkIsOwner), comboPoints(comboPoints) {}
        virtual string GetTargetName() { return "self target"; }
        virtual string GetComboPointsTargetName() { return "current target"; }
        virtual bool IsActive();

    private:
        uint8 comboPoints;
    };

    class NoDebuffAndComboPointsAvailableTrigger : public NoBuffAndComboPointsAvailableTrigger
    {
    public:
        NoDebuffAndComboPointsAvailableTrigger(PlayerbotAI* ai, string spell, uint8 comboPoints, bool checkIsOwner = true, int checkInterval = 1) : NoBuffAndComboPointsAvailableTrigger(ai, spell, comboPoints, checkIsOwner, checkIsOwner) {}
        virtual string GetTargetName() { return "current target"; }
    };

    class ProtectPartyMemberTrigger : public Trigger
    {
    public:
        ProtectPartyMemberTrigger(PlayerbotAI* ai) : Trigger(ai, "protect party member") {}
        virtual string GetTargetName() { return "party member to protect"; }

        virtual bool IsActive()
        {
            return AI_VALUE(Unit*, "party member to protect");
        }
    };

    class NoAttackersTrigger : public Trigger
    {
    public:
        NoAttackersTrigger(PlayerbotAI* ai, string name = "no attackers") : Trigger(ai, name) {}
        virtual bool IsActive();
    };

    class NoTargetTrigger : public Trigger
    {
    public:
        NoTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "no target") {}
        virtual bool IsActive();
    };

    class InvalidTargetTrigger : public Trigger
    {
    public:
        InvalidTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "invalid target") {}
        virtual bool IsActive();
    };

    class TargetInSightTrigger : public Trigger 
    {
    public:
        TargetInSightTrigger(PlayerbotAI* ai) : Trigger(ai, "target in sight") {}
        virtual bool IsActive() { return AI_VALUE(Unit*, "grind target"); }
    };

    class DebuffTrigger : public BuffTrigger
    {
    public:
        DebuffTrigger(PlayerbotAI* ai, string spell, int checkInterval = 1, bool checkIsOwner = false) : BuffTrigger(ai, spell, checkInterval, checkIsOwner) {}

    public:
		virtual string GetTargetName() { return "current target"; }
        virtual bool IsActive();

    protected:
        bool HasMaxDebuffs();
    };

    class DebuffOnAttackerTrigger : public DebuffTrigger
    {
    public:
        DebuffOnAttackerTrigger(PlayerbotAI* ai, string spell) : DebuffTrigger(ai, spell) {}

    public:
        virtual Value<Unit*>* GetTargetValue();
        virtual string getName() { return spell + " on attacker"; }
    };

	class BoostTrigger : public BuffTrigger
	{
	public:
		BoostTrigger(PlayerbotAI* ai, string spell, float balance = 50) : BuffTrigger(ai, spell, 3), balance(balance) {}
		virtual bool IsActive();

	protected:
		float balance;
	};

    class RandomTrigger : public Trigger
    {
    public:
        RandomTrigger(PlayerbotAI* ai, string name, int probability = 7) : Trigger(ai, name)
        {
            this->probability = probability;
            lastCheck = time(0);
        }

        virtual bool IsActive();

    protected:
        int probability;
        time_t lastCheck;
    };

    class TimeTrigger : public Trigger
    {
    public:
        TimeTrigger(PlayerbotAI* ai, string name, int interval = 2) : Trigger(ai, name, interval) {}
        virtual bool IsActive() { return true; }
    };

    class AndTrigger : public Trigger, public Qualified
    {
    public:
         AndTrigger(PlayerbotAI* ai, string name = "and", int checkInterval = 1) : Trigger(ai, name, checkInterval), Qualified() {}
        virtual bool IsActive();
        virtual string getName();
    };

    class OrTrigger : public Trigger, public Qualified
    {
    public:
        OrTrigger(PlayerbotAI* ai, string name = "or", int checkInterval = 1) : Trigger(ai, name, checkInterval), Qualified() {}
        virtual bool IsActive();
        virtual string getName();
    };

    class TwoTriggers : public Trigger
    {
    public:
        TwoTriggers(PlayerbotAI* ai, std::string name1 = "", std::string name2 = "") : Trigger(ai)
        {
            this->name1 = name1;
            this->name2 = name2;
        }

        virtual bool IsActive();
        virtual string getName();

    protected:
        std::string name1;
        std::string name2;
    };

    class ValueTrigger : public Trigger, public Qualified
    {
    public:
        ValueTrigger(PlayerbotAI* ai, string name = "val", int checkInterval = 1) : Trigger(ai, name, checkInterval), Qualified() {}
        virtual bool IsActive() { return AI_VALUE(bool, getQualifier()); }
    };

    class SnareTargetTrigger : public DebuffTrigger
    {
    public:
        SnareTargetTrigger(PlayerbotAI* ai, string spell, int interval = 1) : DebuffTrigger(ai, spell, interval) {}
        virtual string getName() { return spell + " on snare target"; }
        virtual Value<Unit*>* GetTargetValue();
    };

    class NoManaTrigger : public Trigger
    {
    public:
        NoManaTrigger(PlayerbotAI* ai) : Trigger(ai, "no mana") {}

        virtual bool IsActive();
    };

	class LowManaTrigger : public Trigger
	{
	public:
		LowManaTrigger(PlayerbotAI* ai) : Trigger(ai, "low mana") {}

		virtual bool IsActive();
	};

	class MediumManaTrigger : public Trigger
	{
	public:
		MediumManaTrigger(PlayerbotAI* ai) : Trigger(ai, "medium mana") {}

		virtual bool IsActive();
	};

    class HighManaTrigger : public Trigger
    {
    public:
        HighManaTrigger(PlayerbotAI* ai) : Trigger(ai, "high mana") {}

        virtual bool IsActive();
    };

    class AlmostFullManaTrigger : public Trigger
    {
    public:
        AlmostFullManaTrigger(PlayerbotAI* ai) : Trigger(ai, "almost full mana") {}

        virtual bool IsActive();
    };

    BEGIN_TRIGGER(PanicTrigger, Trigger)
        virtual string getName() { return "panic"; }
    END_TRIGGER()

    BEGIN_TRIGGER(OutNumberedTrigger, Trigger)
        virtual string getName() { return "outnumbered"; }
    END_TRIGGER()

	class NoPetTrigger : public Trigger
	{
	public:
		NoPetTrigger(PlayerbotAI* ai) : Trigger(ai, "no pet", 30) {}

		virtual bool IsActive() 
        {
			return !AI_VALUE(Unit*, "pet target") && !AI_VALUE2(bool, "mounted", "self target");
		}
	};

	class ItemCountTrigger : public Trigger 
    {
	public:
		ItemCountTrigger(PlayerbotAI* ai, string item, int count, int interval = 30) : Trigger(ai, item, interval) 
        {
			this->item = item;
			this->count = count;
		}

		virtual bool IsActive();
		virtual string getName() { return "item count"; }

	protected:
		string item;
		int count;
	};

    class AmmoCountTrigger : public ItemCountTrigger
    {
    public:
        AmmoCountTrigger(PlayerbotAI* ai, string item, uint32 count = 1, int interval = 30) : ItemCountTrigger(ai, item, count, interval) {}
    };

	class HasAuraTrigger : public Trigger 
    {
	public:
		HasAuraTrigger(PlayerbotAI* ai, string spell, int interval = 1, int auraTypeId = TOTAL_AURAS) : Trigger(ai, spell, interval), auraTypeId(auraTypeId) {}
		virtual string GetTargetName() { return "self target"; }
		virtual bool IsActive();

    protected:
        int auraTypeId;
	};

    class HasNoAuraTrigger : public Trigger 
    {
    public:
        HasNoAuraTrigger(PlayerbotAI* ai, string spell) : Trigger(ai, spell) {}
        virtual string GetTargetName() { return "self target"; }
        virtual bool IsActive();
    };

    class TimerTrigger : public Trigger
    {
    public:
        TimerTrigger(PlayerbotAI* ai) : Trigger(ai, "timer"), lastCheck(0) {}

        virtual bool IsActive()
        {
            if (time(0) != lastCheck)
            {
                lastCheck = time(0);
                return true;
            }
            return false;
        }

    private:
        time_t lastCheck;
    };

	class TankAssistTrigger : public NoAttackersTrigger
	{
	public:
        TankAssistTrigger(PlayerbotAI* ai) : NoAttackersTrigger(ai, "tank assist") {}
		virtual bool IsActive();
	};

    class IsBehindTargetTrigger : public Trigger
    {
    public:
        IsBehindTargetTrigger(PlayerbotAI* ai) : Trigger(ai) {}
        virtual bool IsActive();
    };

    class IsNotBehindTargetTrigger : public Trigger
    {
    public:
        IsNotBehindTargetTrigger(PlayerbotAI* ai) : Trigger(ai) {}
        virtual bool IsActive();
    };

    class IsNotFacingTargetTrigger : public Trigger
    {
    public:
        IsNotFacingTargetTrigger(PlayerbotAI* ai) : Trigger(ai) {}
        virtual bool IsActive();
    };

    class HasCcTargetTrigger : public Trigger
    {
    public:
        HasCcTargetTrigger(PlayerbotAI* ai, string name) : Trigger(ai, name) {}
        virtual bool IsActive();
    };

	class NoMovementTrigger : public Trigger
	{
	public:
		NoMovementTrigger(PlayerbotAI* ai, string name) : Trigger(ai, name) {}
		virtual bool IsActive();
	};

    class NoPossibleTargetsTrigger : public Trigger
    {
    public:
        NoPossibleTargetsTrigger(PlayerbotAI* ai) : Trigger(ai, "no possible targets") {}
        virtual bool IsActive();
    };

    class NotDpsTargetActiveTrigger : public Trigger
    {
    public:
        NotDpsTargetActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "not dps target active", 2) {}
        virtual bool IsActive();
    };

    class NotDpsAoeTargetActiveTrigger : public Trigger
    {
    public:
        NotDpsAoeTargetActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "not dps aoe target active") {}
        virtual bool IsActive();
    };

    class PossibleAddsTrigger : public Trigger
    {
    public:
        PossibleAddsTrigger(PlayerbotAI* ai) : Trigger(ai, "possible adds") {}
        virtual bool IsActive();
    };

    class IsSwimmingTrigger : public Trigger
    {
    public:
        IsSwimmingTrigger(PlayerbotAI* ai) : Trigger(ai, "swimming") {}
        virtual bool IsActive();
    };

    class HasNearestAddsTrigger : public Trigger
    {
    public:
        HasNearestAddsTrigger(PlayerbotAI* ai) : Trigger(ai, "has nearest adds") {}
        virtual bool IsActive();
    };

    class HasItemForSpellTrigger : public Trigger
    {
    public:
        HasItemForSpellTrigger(PlayerbotAI* ai, string spell) : Trigger(ai, spell) {}
        virtual bool IsActive();
    };

    class TargetChangedTrigger : public Trigger
    {
    public:
        TargetChangedTrigger(PlayerbotAI* ai) : Trigger(ai, "target changed") {}
        virtual bool IsActive();
    };

    class InterruptEnemyHealerTrigger : public SpellTrigger
    {
    public:
        InterruptEnemyHealerTrigger(PlayerbotAI* ai, string spell) : SpellTrigger(ai, spell) {}
        virtual Value<Unit*>* GetTargetValue();
        virtual string getName() { return spell + " on enemy healer"; }
    };

    class RandomBotUpdateTrigger : public RandomTrigger
    {
    public:
        RandomBotUpdateTrigger(PlayerbotAI* ai) : RandomTrigger(ai, "random bot update", 30) {}

        virtual bool IsActive()
        {
            return RandomTrigger::IsActive() && AI_VALUE(bool, "random bot update");
        }
    };

    class NoNonBotPlayersAroundTrigger : public Trigger
    {
    public:
        NoNonBotPlayersAroundTrigger(PlayerbotAI* ai) : Trigger(ai, "no non bot players around", 10) {}

        virtual bool IsActive()
        {
            return !ai->HasPlayerNearby();
            /*if (!bot->InBattleGround())
                return AI_VALUE(list<ObjectGuid>, "nearest non bot players").empty();
            else
                return false;*/
        }
    };

    class NewPlayerNearbyTrigger : public Trigger
    {
    public:
        NewPlayerNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "new player nearby", 10) {}

        virtual bool IsActive()
        {
            return AI_VALUE(ObjectGuid, "new player nearby");
        }
    };

    class CollisionTrigger : public Trigger
    {
    public:
        CollisionTrigger(PlayerbotAI* ai) : Trigger(ai, "collision", 5) {}

        virtual bool IsActive()
        {
            return AI_VALUE2(bool, "collision", "self target");
        }
    };

    class StayTimeTrigger : public Trigger
    {
    public:
        StayTimeTrigger(PlayerbotAI* ai, uint32 delay, string name) : Trigger(ai, name, 5), delay(delay) {}

        virtual bool IsActive();

    private:
        uint32 delay;
    };

    class SitTrigger : public StayTimeTrigger
    {
    public:
        SitTrigger(PlayerbotAI* ai) : StayTimeTrigger(ai, sPlayerbotAIConfig.sitDelay, "sit") {}
    };

    class ReturnTrigger : public StayTimeTrigger
    {
    public:
        ReturnTrigger(PlayerbotAI* ai) : StayTimeTrigger(ai, sPlayerbotAIConfig.returnDelay, "return") {}
    };

    class ReturnToStayPositionTrigger : public Trigger
    {
    public:
        ReturnToStayPositionTrigger(PlayerbotAI* ai) : Trigger(ai, "return to stay position", 2) {}

        virtual bool IsActive();
    };

    class ReturnToPullPositionTrigger : public Trigger
    {
    public:
        ReturnToPullPositionTrigger(PlayerbotAI* ai) : Trigger(ai, "return to pull position", 2) {}

        virtual bool IsActive();
    };

    class GiveItemTrigger : public Trigger
    {
    public:
        GiveItemTrigger(PlayerbotAI* ai, string name, string item) : Trigger(ai, name, 2), item(item) {}

        virtual bool IsActive()
        {
            return AI_VALUE2(Unit*, "party member without item", item) && AI_VALUE2(uint32, "item count", item);
        }

    protected:
        string item;
    };

    class GiveFoodTrigger : public GiveItemTrigger
    {
    public:
        GiveFoodTrigger(PlayerbotAI* ai) : GiveItemTrigger(ai, "give food", "conjured food") {}

        virtual bool IsActive()
        {
            return AI_VALUE(Unit*, "party member without food") && AI_VALUE2(uint32, "item count", item);
        }
    };

    class GiveWaterTrigger : public GiveItemTrigger
    {
    public:
        GiveWaterTrigger(PlayerbotAI* ai) : GiveItemTrigger(ai, "give water", "conjured water") {}

        virtual bool IsActive()
        {
            return AI_VALUE(Unit*, "party member without water") && AI_VALUE2(uint32, "item count", item);
        }
    };

    class IsMountedTrigger : public Trigger
    {
    public:
        IsMountedTrigger(PlayerbotAI* ai) : Trigger(ai, "mounted", 1) {}

    public:
        virtual bool IsActive();
    };

    class CorpseNearTrigger : public Trigger
    {
    public:
        CorpseNearTrigger(PlayerbotAI* ai) : Trigger(ai, "corpse near", 10) {}

    public:
        virtual bool IsActive();
    };

    class IsFallingTrigger : public Trigger
    {
    public:
        IsFallingTrigger(PlayerbotAI* ai) : Trigger(ai, "falling", 10) {}

    public:
        virtual bool IsActive();
    };

    class IsFallingFarTrigger : public Trigger
    {
    public:
        IsFallingFarTrigger(PlayerbotAI* ai) : Trigger(ai, "falling far", 10) {}

    public:
        virtual bool IsActive();
    };

    class UseTrinketTrigger : public BoostTrigger
    {
    public:
        UseTrinketTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "use trinket", 3) {}
        virtual bool IsActive() { return AI_VALUE(list<Item*>, "trinkets on use").size() > 0; }
    };

    class HasAreaDebuffTrigger : public Trigger 
    {
    public:
        HasAreaDebuffTrigger(PlayerbotAI* ai) : Trigger(ai, "has area debuff", 3) {}
        virtual bool IsActive();
    };

    // racials

    class BerserkingTrigger : public BoostTrigger
    {
    public:
        BerserkingTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "berserking") {}
    };

    class BloodFuryTrigger : public BoostTrigger
    {
    public:
        BloodFuryTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "blood fury") {}
    };

    class CannibalizeTrigger : public Trigger
    {
    public:
        CannibalizeTrigger(PlayerbotAI* ai) : Trigger(ai, "cannibalize") {}

        virtual bool IsActive()
        {
            if (AI_VALUE2(uint8, "health", "self target") > sPlayerbotAIConfig.almostFullHealth)
                return false;

            list<ObjectGuid> corpses = context->GetValue<list<ObjectGuid> >("nearest corpses")->Get();
            for (list<ObjectGuid>::iterator i = corpses.begin(); i != corpses.end(); i++)
            {
                if (!i->IsUnit())
                    continue;

                Unit* corpse = ai->GetUnit(*i);
                if (!corpse)
                    continue;

                if (!(corpse->GetCreatureType() == CREATURE_TYPE_HUMANOID || corpse->GetCreatureType() == CREATURE_TYPE_UNDEAD))
                    continue;

                if (sServerFacade.GetDistance2d(bot, corpse) <= 5.0f)
                    return true;
            }
            return false;
        }

    };

    class WOtFTrigger : public Trigger
    {
    public:
        WOtFTrigger(PlayerbotAI* ai) : Trigger(ai, "will of the forsaken") {}

        virtual bool IsActive()
        {
            return bot->HasAuraType(SPELL_AURA_MOD_FEAR);
            return bot->HasAuraType(SPELL_AURA_MOD_STUN);
            return bot->HasAuraType(SPELL_AURA_MOD_CHARM);
            return bot->HasAuraType(SPELL_AURA_MOD_CONFUSE);
        }
    };

    class RootedTrigger : public Trigger
    {
    public:
        RootedTrigger(PlayerbotAI* ai) : Trigger(ai, "rooted") {}

        bool IsActive() override
        {
            return bot->HasAuraType(SPELL_AURA_MOD_ROOT) ||
                   bot->HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED);
        }
    };

    class PartyMemberRootedTrigger : public Trigger
    {
    public:
        PartyMemberRootedTrigger(PlayerbotAI* ai) : Trigger(ai, "party member rooted") {}

        bool IsActive() override
        {
            return AI_VALUE(Unit*, "party member to remove roots");
        }
    };

    class FearedTrigger : public Trigger
    {
    public:
        FearedTrigger(PlayerbotAI* ai) : Trigger(ai, "feared") {}

        bool IsActive() override
        {
            return bot->HasAuraType(SPELL_AURA_MOD_FEAR);
        }
    };

    class StunnedTrigger : public Trigger
    {
    public:
        StunnedTrigger(PlayerbotAI* ai) : Trigger(ai, "stunned") {}

        bool IsActive() override
        {
            return bot->HasAuraType(SPELL_AURA_MOD_STUN);
        }
    };

    class CharmedTrigger : public Trigger
    {
    public:
        CharmedTrigger(PlayerbotAI* ai) : Trigger(ai, "charmed") {}

        bool IsActive() override
        {
            return bot->HasAuraType(SPELL_AURA_MOD_CHARM);
        }
    };

    class StoneformTrigger : public Trigger
    {
    public:
        StoneformTrigger(PlayerbotAI* ai) : Trigger(ai, "stoneform") {}

        virtual bool IsActive()
        {
            uint32 disMask = GetDispellMask(DISPEL_DISEASE);
            uint32 poisMask = GetDispellMask(DISPEL_POISON);
            uint32 bleedType = 1 << (MECHANIC_BLEED - 1);
            for (auto itr : bot->GetSpellAuraHolderMap())
            {
                SpellEntry const* spell = itr.second->GetSpellProto();
                if (IsPositiveSpell(spell->Id))
                    continue;

                if (((1 << spell->Dispel) & disMask) || ((1 << spell->Dispel) & poisMask))
                    return true;

                if (!spell->HasAttribute(SPELL_ATTR_NO_IMMUNITIES) && itr.second->HasMechanicMask(bleedType))
                    return true;
            }
            return false;
        }
    };

    class ShadowmeldTrigger : public Trigger
    {
    public:
        ShadowmeldTrigger(PlayerbotAI* ai) : Trigger(ai, "shadowmeld") {}

        virtual bool IsActive()
        {
            Unit* master = ai->GetMaster();
            if (ai->HasAura("shadowmeld", bot))
                return false;
            return ((!master || (master && master->HasStealthAura() && !master->IsMoving())) && !bot->IsMoving());
        }
    };

    class ManaTapTrigger : public Trigger
    {
    public:
        ManaTapTrigger(PlayerbotAI* ai) : Trigger(ai, "mana tap") {}

        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            return target && AI_VALUE2(bool, "has mana", "current target");
        }
    };

    class ArcanetorrentTrigger : public InterruptSpellTrigger
    {
    public:
        ArcanetorrentTrigger(PlayerbotAI* ai) : InterruptSpellTrigger(ai, "arcane torrent") {}

        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            return InterruptSpellTrigger::IsActive() && target && AI_VALUE2(float, "distance", "current target") <= 8.0f;
        }
    };

    class WarStompTrigger : public Trigger
    {
    public:
        WarStompTrigger(PlayerbotAI* ai) : Trigger(ai, "war stomp") {}

        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            return target && AI_VALUE2(bool, "combat", "self target") && AI_VALUE2(float, "distance", "current target") <= 8.0f &&
                (AI_VALUE2(uint8, "health", "self target") < sPlayerbotAIConfig.mediumHealth ||
                    AI_VALUE(uint8, "my attacker count") >= 3 ||
                    target->IsNonMeleeSpellCasted(true));
        }
    };

    class PerceptionTrigger : public BuffTrigger
    {
    public:
        PerceptionTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "perception") {}

        virtual bool IsActive()
        {
            for (auto& attacker : ai->GetAiObjectContext()->GetValue<list<ObjectGuid>>("possible attack targets")->Get())
            {
                Unit* enemy = ai->GetUnit(attacker);
                if (!enemy)
                    continue;

                if (enemy->getClass() == CLASS_ROGUE || enemy->getClass() == CLASS_DRUID)
                    return true;
            }
            return false;
        }
    };

    class InPvpTrigger : public Trigger
    {
    public:
        InPvpTrigger(PlayerbotAI* ai, std::string name = "in pvp") : Trigger(ai, name, 5) {}
        bool IsActive() override;
    };

    class InPveTrigger : public Trigger
    {
    public:
        InPveTrigger(PlayerbotAI* ai, std::string name = "in pve") : Trigger(ai, name, 5) {}
        bool IsActive() override;
    };

    class InRaidFightTrigger : public Trigger
    {
    public:
        InRaidFightTrigger(PlayerbotAI* ai, std::string name = "in raid fight") : Trigger(ai, name, 5) {}
        bool IsActive() override;
    };

    class HasBlessingOfSalvationTrigger : public HasAuraTrigger
    {
    public:
        HasBlessingOfSalvationTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "blessing of salvation") {}
    };

    class HasGreaterBlessingOfSalvationTrigger : public HasAuraTrigger
    {
    public:
        HasGreaterBlessingOfSalvationTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "greater blessing of salvation") {}
    };

    class BuffOnTargetTrigger : public Trigger
    {
    public:
        BuffOnTargetTrigger(PlayerbotAI* ai, string name, uint32 buffID)
        : Trigger(ai, name, 1)
        , buffID(buffID) {}

        virtual bool IsActive() override;

    protected:
        virtual string GetTargetName() override { return "current target"; }

    protected:
        uint32 buffID;
    };

    class DispelOnTargetTrigger : public Trigger
    {
    public:
        DispelOnTargetTrigger(PlayerbotAI* ai, string name, DispelType dispelType, int checkInterval = 1)
        : Trigger(ai, name, checkInterval)
        , dispelType(dispelType) {}

        virtual bool IsActive() override;

    protected:
        virtual string GetTargetName() override { return "current target"; }

    private:
        DispelType dispelType;
    };

    class DispelEnrageOnTargetTrigger : public DispelOnTargetTrigger
    {
    public:
        DispelEnrageOnTargetTrigger(PlayerbotAI* ai, string name = "dispel enrage") : DispelOnTargetTrigger(ai, name, DISPEL_ENRAGE) {}
    };
}

#include "RangeTriggers.h"
#include "HealthTriggers.h"
#include "CureTriggers.h"
