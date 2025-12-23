#ifndef STARSECTOR_NANO_SHIPPOOL_H
#define STARSECTOR_NANO_SHIPPOOL_H


#include "ShipAPI.h"
#include <stdbool.h>
#include "ExplosionPool.h"
#include "SequencedExplosion.h"

#define MAX_SHIPS 20

typedef struct ShipPool{
    ShipAPI* ships[MAX_SHIPS]; // 固定大小的对象池
    bool inUse[MAX_SHIPS];     // 标记每个对象是否正在使用
    int count;          // 当前使用的对象数量
} ShipPool;

// 初始化对象池
ShipPool* ShipPool_Init();

// 获取一个空闲的ShipAPI对象
ShipAPI* ShipPool_GetShip(ShipPool* pool,ResourceManager* rsm, RenderManager* rdm, const char* name, Vector2f position, float angle, Vector2f velocity, bool isPlayer = false);

// 更新所有已使用的ShipAPI对象
void ShipPool_Update(ResourceManager* rsm, RenderManager* rdm, SequencedExplosionPool* exp, ShipPool* pool, Camera* camera, float deltaTime);

// 渲染所有已使用的ShipAPI对象
void ShipPool_Render(ShipPool* pool, RenderManager* rdm);

// 检查所有已使用的ShipAPI对象之间的碰撞
void ShipPool_CheckCollisions(ShipPool* pool, RenderManager* rdm, float deltaTime);

// 显示船只血量
void ShipPool_Render_Hitpoints(ShipPool* pool, RenderManager* rdm);
#endif //STARSECTOR_NANO_SHIPPOOL_H
