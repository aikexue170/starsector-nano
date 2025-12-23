#ifndef STARSECTOR_NANO_CUSTOMFSM_H
#define STARSECTOR_NANO_CUSTOMFSM_H

#include "ShipAPI.h"
#include "GlobalCombatData.h"
#include "Vector2f.h"
#include "BulletPool.h"

typedef enum {
    FSM_STATE_PATROL,
    FSM_STATE_ATTACK,
    FSM_STATE_EVADE
} FSMState;

typedef struct {
    FSMState currentState;
    float stateTimer;
    int targetId;
    Vector2f lastKnownEnemyPos;
} FSMContext;

typedef struct {
    ExplosionPool* expPool;
    BulletPool* bulletPool;
    RenderManager* renderMgr;
    ResourceManager* resMgr;
    double deltaTime;
} FSMResources;

void FSM_Init(FSMContext* ctx);
void FSM_Update(ShipAPI* ship, FSMContext* ctx,
                const CombatDataPool* combatData,
                const FSMResources* res);
#endif //STARSECTOR_NANO_CUSTOMFSM_H
