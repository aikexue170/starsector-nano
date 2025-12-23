#include "CustomFSM.h"
#include <math.h>
#include <float.h>

// 私有工具函数
static int FindNearestValidEnemy(const ShipAPI* ship, const CombatDataPool* data) {
    int nearestId = -1;
    float minDist = 1200.0f; // 探测范围

    for (int i = 0; i < MAX_SHIPS; i++) {
        if (!data->data[i].active || data->data[i].team == ship->team){
            if(data->data[i].team == ship->team){
            }
            continue;
        }


        // 排除自身（精确位置比对）
        Vector2f selfPos = ship->entity_data.position;
        Vector2f targetPos = data->data[i].position;
        if (fabs(selfPos.x - targetPos.x) < FLT_EPSILON &&
            fabs(selfPos.y - targetPos.y) < FLT_EPSILON) {
            continue;
        }

        float dist;
        Vector2f delta;
        Vector2f_subtract(&selfPos, &targetPos, &delta);
        dist = Vector2f_length(&delta);

        if (dist < minDist) {
            minDist = dist;
            nearestId = i;
        }
    }
    return (minDist <= 2000.0f) ? nearestId : -1;
}

// 状态处理逻辑
static void HandlePatrol(ShipAPI* ship, const FSMResources* res) {
    // 默认前进行为
    ShipAPI_Forward(ship, res->deltaTime);
}

static void HandleAttack(ShipAPI* ship, FSMContext* ctx,
                         const CombatDataPool* data,
                         const FSMResources* res) {
    if (ctx->targetId == -1 || !data->data[ctx->targetId].active) {
        ctx->currentState = FSM_STATE_PATROL;
        return;
    }

    ship->aim_position = data->data[ctx->targetId].position;

    // 计算距离
    Vector2f delta;
    Vector2f_subtract(&ship->entity_data.position, &data->data[ctx->targetId].position, &delta);
    float dist = Vector2f_length(&delta);

    // 计算目标方向向量
    Vector2f toTarget;
    Vector2f_subtract(&data->data[ctx->targetId].position, &ship->entity_data.position, &toTarget);

    // 计算当前朝向角度和目标角度（弧度）
    float currentAngle = ship->entity_data.angle;
    float targetAngle = 90 + fmodf((atan2f(toTarget.y, toTarget.x) * 180.0f / PI) + 360.0f, 360.0f);

    // 计算最小角度差（考虑2π环绕）
    float angleDiff = targetAngle - currentAngle;

    // 转向控制（保持不变）
    if (angleDiff > 20) {  // 需要左转
        ShipAPI_TurnRight(ship, res->deltaTime);
    }
    else if (angleDiff < -20) {  // 需要右转
        ShipAPI_TurnLeft(ship, res->deltaTime);
    }

    // 改进的油门控制逻辑
    if (angleDiff > 150 || angleDiff < -150) {  // 角度差 > ~180度（屁股对着敌人）
        // 后退以保持距离
        if (dist > 300.0f) {

        } else {
            ShipAPI_Backward(ship, res->deltaTime);  // 后退逼近
        }
    }
    else if (angleDiff > 60 || angleDiff < -60) {  // 角度差 > 60度

    }
    else {
        // 正常距离控制
        float desiredDistance = 400.0f;  // 理想战斗距离
        float distanceThreshold = 50.0f;  // 距离缓冲范围

        if (dist > desiredDistance + distanceThreshold) {
            ShipAPI_Forward(ship, res->deltaTime);
        }
        else if (dist < desiredDistance - distanceThreshold) {
            ShipAPI_Backward(ship, res->deltaTime);
        }
        else {
            ShipAPI_EngineDown(ship);
        }
    }

    // 开火条件：距离<1000且角度差<45度
    if (dist < 1000.0f && (angleDiff <45 || angleDiff > -45)) {
        ShipAPI_Fire(ship, res->expPool, res->bulletPool, res->renderMgr, res->resMgr);
    }
    if (dist < 700) {
        ShipAPI_Fire(ship, res->expPool, res->bulletPool, res->renderMgr, res->resMgr);
    }
}

// 公开接口
void FSM_Init(FSMContext* ctx) {
    ctx->currentState = FSM_STATE_PATROL;
    ctx->stateTimer = 0;
    ctx->targetId = -1;
    Vector2f_init(&ctx->lastKnownEnemyPos, 0, 0);
}

void FSM_Update(ShipAPI* ship, FSMContext* ctx,
                const CombatDataPool* combatData,
                const FSMResources* res) {
    // 状态机更新
    switch (ctx->currentState) {
        case FSM_STATE_PATROL:
            HandlePatrol(ship, res);
            ctx->targetId = FindNearestValidEnemy(ship, combatData);
            if (ctx->targetId != -1) {
                ctx->currentState = FSM_STATE_ATTACK;
            }
            break;

        case FSM_STATE_ATTACK:
            ctx->targetId = FindNearestValidEnemy(ship, combatData);
            if (ctx->targetId != -1) {
                HandleAttack(ship, ctx, combatData, res);
            } else {
                ctx->currentState = FSM_STATE_PATROL;
            }
            break;

        default:
            break;
    }
}