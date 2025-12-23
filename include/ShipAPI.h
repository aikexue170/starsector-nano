#ifndef STARSECTOR_NANO_SHIPAPI_H
#define STARSECTOR_NANO_SHIPAPI_H
#include "Engine.h"
#include "WeaponAPI.h"


typedef struct ShipAPI {
    Entity entity_data;
    Engine engine[5];
    int engine_count;
    WeaponAPI weapon[10];
    int weapon_count;
    bool isPlayer;
    int team = 0;
    Vector2f aim_position = {0, 0};
} ShipAPI;

ShipAPI* ShipAPI_Init(RenderManager* rdm,
                  ResourceManager* rm,
                  const char* name,
                  Vector2f position,
                  float angle,
                  Vector2f velocity,
                  bool isPlayer = false);

void ShipAPI_AddTeam(ShipAPI* ship, int team);

void ShipAPI_Update(ShipAPI* ship, Camera* camera, float deltaTime, Vector2f position = {0, 0});

void ShipAPI_Render(RenderManager* rdm, ShipAPI* ship);

void ShipAPI_Forward(ShipAPI* ship, float deltaTime);

void ShipAPI_Backward(ShipAPI* ship, float deltaTime);

void ShipAPI_TurnLeft(ShipAPI* ship, float deltaTime);

void ShipAPI_TurnRight(ShipAPI* ship, float deltaTime);

void ShipAPI_EngineDown(ShipAPI* ship);

void ShipAPI_Destroy(ResourceManager* rsm, RenderManager* rdm, ShipAPI* ship);


/*
 * 怕自己忘了，写写注释，关于状态机AI如何调用船只的接口
void ShipAPI_Forward(ShipAPI* ship, float deltaTime);

void ShipAPI_Backward(ShipAPI* ship, float deltaTime);

void ShipAPI_TurnLeft(ShipAPI* ship, float deltaTime);

void ShipAPI_TurnRight(ShipAPI* ship, float deltaTime);
 这四个是前进后退左转右转

武器的瞄准：
 ShipAPI_Update
 最后传入要瞄准的position

武器发射：ShipAPI_Fire函数
 */


#endif //STARSECTOR_NANO_SHIPAPI_H
