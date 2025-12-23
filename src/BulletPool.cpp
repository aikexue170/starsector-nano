#include "BulletPool.h"

BulletPool* BulletPool_Init(){
    BulletPool* pool = new(BulletPool);
    pool->count = 0;
    for (int i = 0; i < MAX_BULLETS; ++i) {
        pool->bullet[i] = NULL;
        pool->inUse[i] = false;
    }
    return pool;
}

bool BulletPool_GenerateBullet(BulletPool* pool,
                               ResourceManager* rsm,
                               RenderManager* rdm,
                               const char* name,
                               WeaponAPI* weapon) {
    // 查找第一个空闲位置
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!pool->inUse[i]) {
            // 创建新子弹
            BulletAPI* bullet = BulletAPI_Init(rdm, rsm, name, weapon);
            if(!bullet) {
                fprintf(stderr, "Error: Failed to create bullet\n");
                return false;
            }

            // 如果该位置有旧子弹，先销毁
            if(pool->bullet[i]) {
                BulletAPI_Destroy(rdm, pool->bullet[i]);
            }

            pool->bullet[i] = bullet;
            pool->inUse[i] = true;
            pool->count++;
            return true;
        }
    }

    fprintf(stderr, "Error: Bullet pool is full\n");
    return false;
}

// 更新所有已使用的BulletAPI对象
void BulletPool_Update(BulletPool* pool, RenderManager* rdm, float deltaTime) {
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (pool->inUse[i]) {
            BulletAPI_Update(pool->bullet[i], deltaTime);
            if(pool->bullet[i]->out){
                BulletPool_DestroyBullets(pool, rdm, i);
            }
        }
    }
}

// 渲染所有已使用的BulletAPI对象
void BulletPool_Render(BulletPool* pool, RenderManager* rdm) {
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (pool->inUse[i]) {
            BulletAPI_Render(rdm, pool->bullet[i]);
        }
    }
}

void BulletPool_DestroyBullets(BulletPool* pool, RenderManager* rdm, int pool_id) {
    if(!pool || !rdm || pool_id < 0 || pool_id >= MAX_BULLETS) return;

    if(pool->inUse[pool_id] && pool->bullet[pool_id]) {
        BulletAPI_Destroy(rdm, pool->bullet[pool_id]);
        pool->bullet[pool_id] = NULL;
        pool->inUse[pool_id] = false;
        pool->count--;
    }
}

// 检查对象池中所有炮弹的碰撞
void BulletPool_CheckCollisions(BulletPool* pool, RenderManager* rdm, float deltaTime) {
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (pool->inUse[i]) {
            for (int j = i + 1; j < MAX_BULLETS; ++j) {
                if (pool->inUse[j]) {
                    if(!pool->bullet[i]->noCollision && !pool->bullet[j]->noCollision){
                        if (Entity_Collision(&pool->bullet[i]->entity_data, &pool->bullet[j]->entity_data, deltaTime)) {
                            BulletPool_DestroyBullets(pool, rdm, i);
                            BulletPool_DestroyBullets(pool, rdm, j);
                        }
                    }
                }
            }
        }
    }
}

// 检测炮弹和船的碰撞
void Pool_CheckMutualCollisions(BulletPool* bullet_pool, ShipPool* ship_pool, ExplosionPool* exp_pool, ResourceManager* rsm, RenderManager* rdm, float deltaTime){
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (ship_pool->inUse[i]) {
            for (int j = 0; j < MAX_BULLETS; ++j) {
                if (bullet_pool->inUse[j]) {
                    if(!bullet_pool->bullet[j]->noCollision){
                        if(bullet_pool->bullet[j]->original_ship_id != ship_pool->ships[i]->entity_data.id){
                            if (Entity_Collision(&ship_pool->ships[i]->entity_data, &bullet_pool->bullet[j]->entity_data, deltaTime)) {
                                RenderManager_ShakeScreen(rdm, 10);
                                ExplosionPool_GenerateExplosion(exp_pool, rsm, rdm, bullet_pool->bullet[j]->explosionType, bullet_pool->bullet[j]->entity_data.position, 2);
                                BulletPool_Damage(ship_pool->ships[i], bullet_pool->bullet[j]);
                                BulletPool_DestroyBullets(bullet_pool, rdm, j);
                            }
                        }
                    }
                }
            }
        }
    }
}


void WeaponAPI_fire(ExplosionPool* exp_pool, BulletPool* pool, RenderManager* rdm, ResourceManager* rsm, WeaponAPI* weapon){
    if (WeaponAPI_CanFire(weapon)){
        BulletPool_GenerateBullet(pool, rsm, rdm, weapon->bullet_name, weapon);
        if(weapon->isBullet){
            ExplosionPool_GenerateExplosion(exp_pool, rsm, rdm, EXPLOSION_FIRE, weapon->fire_position, 0.5);
        }
        if(weapon->isLazer){
            ExplosionPool_GenerateExplosion(exp_pool, rsm, rdm, EXPLOSION_LASER_FIRE, weapon->fire_position, 0.5);
        }
        weapon->fire_manager.canFire = false;
        weapon->fire_manager.cooldown_time = weapon->cooldown;
    }
}

void ShipAPI_Fire(ShipAPI* ship, ExplosionPool* exp_pool, BulletPool* pool, RenderManager* rdm, ResourceManager* rsm){
    for(int i = 0; i < ship->weapon_count; i++){
        WeaponAPI_fire(exp_pool, pool, rdm, rsm, &ship->weapon[i]);
    }
}

void BulletPool_Damage(ShipAPI* ship, BulletAPI* bullet){
    ship->entity_data.hitpoint -= bullet->damage;
}

void BulletPool_Free(BulletPool* pool, RenderManager* rdm) {
    if(!pool) return;

    // 销毁所有子弹
    for(int i = 0; i < MAX_BULLETS; i++) {
        if(pool->bullet[i]) {
            BulletAPI_Destroy(rdm, pool->bullet[i]);
            pool->bullet[i] = NULL;
        }
    }

    // 释放对象池本身
    free(pool);
}