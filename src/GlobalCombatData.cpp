#include "GlobalCombatData.h"

CombatDataPool* CombatDataPool_Init(){
    CombatDataPool* pool = new(CombatDataPool);
    return pool;
}

void CombatDataPool_Write(CombatDataPool* pool, int id,
                          Vector2f position, float hp, float angle,
                          int team, bool active) {
    if (!pool || id < 0 || id >= MAX_SHIPS) return; // 边界检查

    pool->data[id].position = position;
    pool->data[id].hp = hp;
    pool->data[id].angle = angle;
    pool->data[id].team = team;
    pool->data[id].active = active;
}

void CombatDataPool_Update(CombatDataPool* pool, ShipPool* ship_pool) {
    if (!pool || !ship_pool) return;

    for (int i = 0; i < MAX_SHIPS; i++) {
        // 只处理活跃且有效的船只
        if (ship_pool->inUse[i]) {
            CombatDataPool_Write(
                    pool,
                    i,
                    ship_pool->ships[i]->entity_data.position,
                    ship_pool->ships[i]->entity_data.hitpoint,
                    ship_pool->ships[i]->entity_data.angle,
                    ship_pool->ships[i]->team,
                    true // 标记为活跃
            );
        } else {
            // 非活跃槽位显式标记
            CombatDataPool_Write(
                    pool,
                    i,
                    {0, 0}, // 默认位置
                    0,      // 默认血量
                    0,      // 默认角度
                    0,      // 默认队伍
                    false   // 标记为非活跃
            );
        }
    }
}