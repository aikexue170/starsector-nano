#ifndef STARSECTOR_NANO_GLOBALCOMBATDATA_H
#define STARSECTOR_NANO_GLOBALCOMBATDATA_H

#include "ShipAPI.h"
#include "ShipPool.h"

typedef struct CombatDataPool {
    struct {
        Vector2f position = {0, 0};      // 只读坐标（建议使用 const 或只读接口）
        float hp = 0;             // 只读血量
        float angle;              // 只读角度
        int team = 0;         // 只读阵营
        bool active = false;          // 对应 ShipPool.inUse
    } data[MAX_SHIPS];         // 与 ShipPool 相同大小
} CombatDataPool;

CombatDataPool* CombatDataPool_Init();

void CombatDataPool_Update(CombatDataPool* pool, ShipPool* ship_pool);

void CombatDataPool_Write(CombatDataPool* pool,
                           int id,
                           Vector2f position,
                           float hp,
                           float angle,
                           int team,
                           bool active);

#endif //STARSECTOR_NANO_GLOBALCOMBATDATA_H
