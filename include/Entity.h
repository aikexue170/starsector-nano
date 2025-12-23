#ifndef STARSECTOR_NANO_ENTITY_H
#define STARSECTOR_NANO_ENTITY_H

#include "easyx.h"
#include "Vector2f.h"
#include "Polygon.h"
#include <math.h>
#include <cstdio>
#include <iostream>
#include "Manager.h"
#include "ImageProcessing.h"

// 定义最大旋转速度和线性速度的阈值
#define MAX_ROTATIONAL_VELOCITY 50000.0f  // 根据实际情况调整
#define MAX_LINEAR_SPEED 1000000.0f        // 根据实际情况调整

typedef struct LightController{
    IMAGE light_image;
    IMAGE light_image_after_adjust;
    float intensity;// 0 到 1
}LightController;

typedef struct {
    IMAGE image;
    IMAGE image_with_light;
    HitPolygon* polygon; // JSON读取，不用CSV
    Vector2f position; // 实时更新的位置
    Vector2f velocity; // 实时更新的速度
    Vector2f image_center_offset;  // CSV
    float rotational_velocity; // 实时更新的角速度
    float moment_of_inertia; // 转动惯量
    float acceleration;
    float rotational_acceleration;
    float angle; // 实时更新的角度
    int hitpoint;
    float mass;
    int id; // 渲染器分配id
    Vector2f* transformed_vertices;  // 实时更新的边界数组
    const char* id_name;
    LightController light_controller;
} Entity;


void Entity_Create_FromCSV(ResourceManager* rsm,
                           Entity* entity,
                           HitPolygon* polygon,
                           Vector2f position,
                           float angle,
                           Vector2f velocity,
                           const char* id_name,
                           ResourceType resourceType);

void Entity_AddId(Entity* entity, int id);
// 推进逻辑（前进/后退）
void Entity_Forward(Entity* entity, float deltaTime);
void Entity_Backward(Entity* entity, float deltaTime);
// 旋转逻辑（左转/右转）
void Entity_RotateLeft(Entity* entity, float deltaTime);
void Entity_RotateRight(Entity* entity, float deltaTime);
// 每帧更新位置和角度的通用函数
void Entity_Update(Entity* entity, float deltaTime);
void Entity_Update_Unimpeded(Entity* entity, float deltaTime);
// 碰撞箱可视化
void Entity_ShowPolygon(Entity* entity);
// 碰撞效果
bool Entity_Collision(Entity* entity, Entity* otherEntity, float deltaTime);
// 光效
void Entity_Light(Entity* entity);
// 销毁
void Entity_Destroy(Entity* entity);
#endif //STARSECTOR_NANO_ENTITY_H
