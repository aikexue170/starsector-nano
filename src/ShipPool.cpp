#include "ShipPool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 初始化对象池
ShipPool* ShipPool_Init() {
    ShipPool* pool = new(ShipPool);
    pool->count = 0;
    for (int i = 0; i < MAX_SHIPS; ++i) {
        pool->ships[i] = NULL;
        pool->inUse[i] = false;
    }
    return pool;
}

// 获取一个空闲的ShipAPI对象
ShipAPI* ShipPool_GetShip(ShipPool* pool,ResourceManager* rsm, RenderManager* rdm, const char* name, Vector2f position, float angle, Vector2f velocity, bool isPlayer) {
    // 查找第一个空闲的对象
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (!pool->inUse[i]) {
            pool->ships[i] = ShipAPI_Init(rdm, rsm, name, position, angle, velocity, isPlayer);
            // 设置对象为正在使用状态
            pool->inUse[i] = true;
            pool->count++;
            return pool->ships[i];
        }
    }
    // 如果没有空闲的对象，则返回NULL
    fprintf(stderr, "Error: No available ship objects\n");
    return NULL;
}

// 更新所有已使用的ShipAPI对象
void ShipPool_Update(ResourceManager* rsm, RenderManager* rdm, SequencedExplosionPool* exp, ShipPool* pool, Camera* camera, float deltaTime) {
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (pool->inUse[i]) {
            ShipAPI_Update(pool->ships[i], camera, deltaTime, pool->ships[i]->aim_position);

            if (pool->ships[i]->entity_data.hitpoint <= 0) {
                RenderManager_ScreenFlush();
                ExplosionSequence* seq = SequencedExplosionPool_Acquire(exp);
                if (seq) {
                    ExplosionSequence_Start(
                            seq,
                            pool->ships[i]->entity_data.position,  // 爆炸中心
                            200,           // 爆炸半径
                            5, 8,           // 5~8次爆炸
                            0.15f, 0.75f,    // 爆炸间隔 0.05~0.15秒
                            0.5f,            // 单次爆炸持续时间
                            EXPLOSION_SHIP,  // 爆炸类型
                            false            // 不循环
                    );
                }

                // 销毁飞船
                ShipAPI_Destroy(rsm, rdm, pool->ships[i]);
                pool->ships[i] = NULL;
                pool->inUse[i] = false;
            }
        }
    }
}

// 渲染所有已使用的ShipAPI对象
void ShipPool_Render(ShipPool* pool, RenderManager* rdm) {
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (pool->inUse[i]) {
            ShipAPI_Render(rdm, pool->ships[i]);
        }
    }
}

// 检查对象池中所有船只的碰撞
void ShipPool_CheckCollisions(ShipPool* pool, RenderManager* rdm, float deltaTime) {
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (pool->inUse[i]) {
            for (int j = i + 1; j < MAX_SHIPS; ++j) {
                if (pool->inUse[j]) {
                    if (Entity_Collision(&pool->ships[i]->entity_data, &pool->ships[j]->entity_data, deltaTime)) {
                        RenderManager_ShakeScreen(rdm, 1); // 发生碰撞时震动屏幕
                    }
                }
            }
        }
    }
}
