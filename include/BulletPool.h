#ifndef STARSECTOR_NANO_BULLETPOOL_H
#define STARSECTOR_NANO_BULLETPOOL_H

#include "BulletAPI.h"
#include "ShipPool.h"
#include "ExplosionPool.h"

#define MAX_BULLETS 1000

typedef struct BulletPool{
    BulletAPI* bullet[MAX_BULLETS]; // 固定大小的对象池
    bool inUse[MAX_BULLETS];     // 标记每个对象是否正在使用
    int count;          // 当前使用的对象数量
} BulletPool;

// 初始化对象池
BulletPool* BulletPool_Init();

// 获取一个空闲的BulletAPI对象
bool BulletPool_GenerateBullet(BulletPool* pool,
                               ResourceManager* rsm,
                               RenderManager* rdm,
                               const char* name,
                               WeaponAPI* weapon);

// 更新所有已使用的BulletAPI对象
void BulletPool_Update(BulletPool* pool, RenderManager* rdm, float deltaTime);

// 渲染所有已使用的BulletAPI对象
void BulletPool_Render(BulletPool* pool, RenderManager* rdm);

// 检查所有已使用的BulletAPI对象之间的碰撞
void BulletPool_CheckCollisions(BulletPool* pool, RenderManager* rdm, float deltaTime);

// 检查船只和子弹之间的碰撞
void Pool_CheckMutualCollisions(BulletPool* bullet_pool, ShipPool* ship_pool, ExplosionPool* exp_pool, ResourceManager* rsm, RenderManager* rdm, float deltaTime);

// 销毁子弹
void BulletPool_DestroyBullets(BulletPool* pool, RenderManager* rdm, int pool_id);

void BulletPool_Damage(ShipAPI* ship, BulletAPI* bullet);

void BulletPool_Free(BulletPool* pool, RenderManager* rdm);
// 其他库中的代码
void WeaponAPI_fire(ExplosionPool* exp_pool, BulletPool* pool, RenderManager* rdm, ResourceManager* rsm, WeaponAPI* weapon);

void ShipAPI_Fire(ShipAPI* ship, ExplosionPool* exp_pool, BulletPool* pool, RenderManager* rdm, ResourceManager* rsm);

#endif //STARSECTOR_NANO_BULLETPOOL_H
