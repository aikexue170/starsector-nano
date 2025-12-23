#ifndef STARSECTOR_NANO_WEAPONAPI_H
#define STARSECTOR_NANO_WEAPONAPI_H

#include "easyx.h"
#include "Vector2f.h"
#include "Manager.h"
#include <math.h> // 引入数学库以使用atan2和fabs

// 常数定义
#define PI 3.14159265358979323846

typedef struct FireManager {
    bool canFire = false;
    double cooldown_time;
} FireManager;

typedef struct WeaponAPI {
    // 单个武器的数据资料
    // 类型
    bool isLazer = false;
    bool isBullet = false;
    bool isMissile = false;
    // 用作CSV和JSON读取的name
    const char* name = NULL;
    const char* bullet_name = NULL;
    //渲染器中分配的id
    int id = 0;
    Vector2f original_position = {0, 0};
    Vector2f position = {0, 0};
    Vector2f original_fire_position = {0, 0};
    Vector2f fire_position = {0, 0};
    float angle = 0;
    float final_angle = 0;
    float rotate_acceleration = 0;
    float cooldown = 0;
    float range = 0;
    Vector2f offset = {0, 0};
    IMAGE image = NULL;
    IMAGE glow = NULL;
    IMAGE image_with_glow = NULL;
    // 未来可能加烟尘特效？留一个final的位置吧
    IMAGE image_final_render = NULL;
    // 开火控制器
    FireManager fire_manager = {false, 0};
    int original_ship_id = 0;
    bool isFixed = false;

};

WeaponAPI* WeaponAPI_create_fromCSV(ResourceManager* rsm, const char* name);

void WeaponAPI_UpdateFireManager(WeaponAPI* weapon, float deltaTime);

bool WeaponAPI_CanFire(WeaponAPI* weapon);

void WeaponAPI_FollowMouse(WeaponAPI* weapon, Camera* camera);

void WeaponAPI_Aim(WeaponAPI* weapon, Vector2f position);
// 有一部分代码因为循环引用的问题，被我拆分到BulletPool了，主要是开火相关的逻辑

#endif //STARSECTOR_NANO_WEAPONAPI_H
