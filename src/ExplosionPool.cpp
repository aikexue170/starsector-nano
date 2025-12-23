#include "ExplosionPool.h"

ExplosionPool* ExplosionPool_Init() {
    ExplosionPool* pool = new(ExplosionPool);
    pool->count = 0;
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        pool->explosion[i] = NULL;
        pool->inUse[i] = false;
        pool->id[i] = -1;
    }
    pthread_mutex_init(&pool->mutex, NULL);
    return pool;
}

bool ExplosionPool_GenerateExplosion(ExplosionPool* pool,
                                     ResourceManager* rsm,
                                     RenderManager* rdm,
                                     ExplosionType type,
                                     Vector2f pos,
                                     float duration) {
    pthread_mutex_lock(&pool->mutex);

    bool result = false;
    for(int i = 0; i < MAX_EXPLOSIONS; i++){
        if(!pool->inUse[i] && pool->id[i] != -1){
            ExplosionAPI_ResetExplosion(rsm, pool->explosion[i], pool->id[i], type, pos, duration);
            pool->inUse[i] = true;
            pool->count++;
            result = true;
            break;
        }
        if(pool->id[i] == -1){
            pool->explosion[i] = ExplosionAPI_CreateExplosion(rsm, rdm, type, pos, duration);
            pool->inUse[i] = true;
            pool->count++;
            pool->id[i] = pool->explosion[i]->id;
            result = true;
            break;
        }
    }

    pthread_mutex_unlock(&pool->mutex);
    return result;
}

void ExplosionPool_UpdateMT(ExplosionPool* pool, float deltaTime) {
    pthread_mutex_lock(&pool->mutex);

    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (pool->inUse[i] && pool->explosion[i]->is_active) {
            ExplosionPool_AddTask(pool->explosion[i], deltaTime);
        }
    }

    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (pool->inUse[i] && !pool->explosion[i]->is_active) {
            ExplosionAPI_ClearExplosion(pool->explosion[i]);
            pool->inUse[i] = false;
            pool->count--;
        }
    }

    pthread_mutex_unlock(&pool->mutex);
}

void ExplosionPool_Update(ExplosionPool* pool, RenderManager* rdm, float deltaTime) {
    pthread_mutex_lock(&pool->mutex);

    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (pool->inUse[i]) {
            if(!ExplosionAPI_UpdateExplosion(pool->explosion[i], deltaTime)){
                ExplosionAPI_ClearExplosion(pool->explosion[i]);
                pool->inUse[i] = false;
                pool->count--;
            }
        }
    }

    pthread_mutex_unlock(&pool->mutex);
}

void ExplosionPool_Render(ExplosionPool* pool, RenderManager* rdm) {
    pthread_mutex_lock(&pool->mutex);

    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (pool->inUse[i]) {
            ExplosionAPI_RenderExplosion(pool->explosion[i], rdm);
        }
    }

    pthread_mutex_unlock(&pool->mutex);
}