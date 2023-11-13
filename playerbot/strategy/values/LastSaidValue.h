#pragma once
#include "../Value.h"

namespace ai
{
    class LastSaidValue : public ManualSetValue<time_t>, public Qualified
	{
	public:
        LastSaidValue(PlayerbotAI* ai) : ManualSetValue<time_t>(ai, time(0) - 120, "last said"), Qualified() {}
    };

    class LastEmoteValue : public ManualSetValue<time_t>, public Qualified
	{
	public:
        LastEmoteValue(PlayerbotAI* ai) : ManualSetValue<time_t>(ai, time(0) - 120, "last emote"), Qualified() {}
    };

    class PrevDialogueValue : public ManualSetValue<std::string>, public Qualified
    {
    public:
        PrevDialogueValue(PlayerbotAI* ai) : ManualSetValue<std::string>(ai, "", "prev dialogue"), Qualified() {}
    };
}
