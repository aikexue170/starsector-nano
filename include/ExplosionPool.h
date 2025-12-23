#ifndef STARSECTOR_NANO_EXPLOSIONPOOL_H
#define STARSECTOR_NANO_EXPLOSIONPOOL_H

#include "ExplosionAPI.h"
#include "ThreadPool.h"  // 包含线程池头文件

#define MAX_EXPLOSIONS 1000

typedef struct ExplosionPool {
    ExplosionAPI* explosion[MAX_EXPLOSIONS];
    bool inUse[MAX_EXPLOSIONS];
    int id[MAX_EXPLOSIONS];
    int count;
    pthread_mutex_t mutex;
} ExplosionPool;

// 移除线程池相关声明，因为它们已经在ThreadPool.h中声明

ExplosionPool* ExplosionPool_Init();
bool ExplosionPool_GenerateExplosion(ExplosionPool* pool,
                                     ResourceManager* rsm,
                                     RenderManager* rdm,
                                     ExplosionType type,
                                     Vector2f pos,
                                     float duration);
void ExplosionPool_UpdateMT(ExplosionPool* pool, float deltaTime);
void ExplosionPool_Update(ExplosionPool* pool, RenderManager* rdm, float deltaTime);
void ExplosionPool_Render(ExplosionPool* pool, RenderManager* rdm);

#endif //STARSECTOR_NANO_EXPLOSIONPOOL_H