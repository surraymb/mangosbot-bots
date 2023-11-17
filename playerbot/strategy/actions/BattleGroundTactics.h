#pragma once

#include "MovementActions.h"
#include "BattleGround.h"
#include "BattleGroundMgr.h"
#include "BattleGroundWS.h"
#include "BattleGroundAV.h"
#include "BattleGroundAB.h"
#ifndef MANGOSBOT_ZERO
#include "BattleGroundEY.h"
#endif
#ifdef MANGOSBOT_TWO
#include "BattleGroundIC.h"
#include "BattleGroundSA.h"
#endif
#include "CheckMountStateAction.h"

using namespace ai;

#define SPELL_CAPTURE_BANNER 21651

typedef void(*BattleBotWaypointFunc) ();

// from vmangos
struct BattleBotWaypoint
{
    BattleBotWaypoint(float x_, float y_, float z_, BattleBotWaypointFunc func) :
        x(x_), y(y_), z(z_), pFunc(func) {};
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    BattleBotWaypointFunc pFunc = nullptr;
};

typedef std::vector<BattleBotWaypoint> BattleBotPath;

extern std::vector<BattleBotPath*> const vPaths_WS;
extern std::vector<BattleBotPath*> const vPaths_AB;
extern std::vector<BattleBotPath*> const vPaths_AV;
extern std::vector<BattleBotPath*> const vPaths_EY;
extern std::vector<BattleBotPath*> const vPaths_IC;

class BGTactics : public MovementAction
{
public:
    BGTactics(PlayerbotAI* ai, string name = "bg tactics") : MovementAction(ai, name) {}
    virtual bool Execute(Event& event);
private:
    bool moveToStart(bool force = false);
    bool selectObjective(bool reset = false);
    bool moveToObjective();
    bool selectObjectiveWp(std::vector<BattleBotPath*> const& vPaths);
    bool moveToObjectiveWp(BattleBotPath* const& currentPath, uint32 currentPoint, bool reverse = false);
    bool startNewPathBegin(std::vector<BattleBotPath*> const& vPaths);
    bool startNewPathFree(std::vector<BattleBotPath*> const& vPaths);
    bool resetObjective();
    bool wsgPaths();
    bool wsgRoofJump();
    bool ABSwimPath();
    bool eotsJump();
    bool atFlag(std::vector<BattleBotPath*> const& vPaths, std::vector<uint32> const& vFlagIds);
    bool flagTaken();
    bool teamFlagTaken();
    bool protectFC();
    bool useBuff();
    uint32 getDefendersCount(Position point, float range, bool combat = true);
    bool IsLockedInsideKeep();
};

class ArenaTactics : public MovementAction
{
public:
    ArenaTactics(PlayerbotAI* ai, string name = "arena tactics") : MovementAction(ai, name) {}
    virtual bool Execute(Event& event);
private:
    bool moveToCenter(BattleGround *bg);
};
