#ifndef STARSECTOR_NANO_BULLETAPI_H
#define STARSECTOR_NANO_BULLETAPI_H

#include "Entity.h"
#include "WeaponAPI.h"
#include "Timer.h"
#include "ExplosionAPI.h"

typedef struct BulletAPI {
    Entity entity_data;
    Timer timer;
    bool noCollision = true;
    bool out = false;
    int original_ship_id;
    bool onHit = false;
    ExplosionType explosionType;
    int damage = 0;
};

BulletAPI* BulletAPI_Init(RenderManager* rdm,
                          ResourceManager* rsm,
                          const char* name,
                          WeaponAPI* weapon);


void BulletAPI_Update(BulletAPI* bullet, float deltaTime);

void BulletAPI_Render(RenderManager* rdm, BulletAPI* bullet);

void BulletAPI_Destroy(RenderManager* rm, BulletAPI* bullet);
#endif //STARSECTOR_NANO_BULLETAPI_H
