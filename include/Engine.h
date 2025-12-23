#ifndef STARSECTOR_NANO_ENGINE_H
#define STARSECTOR_NANO_ENGINE_H

#include "Entity.h"
#include "easyx.h"
#include "Vector2f.h"
#include "Distortion.h"
#include "Manager.h"
#include "Timer.h"


typedef struct EngineController{
    float zoom_x;
    float zoom_y;
} EngineController;

typedef struct Engine{
    Vector2f position;
    Vector2f offset_position;
    float angle;
    float offset_angle;
    IMAGE image;
    IMAGE image_after_render;
    IMAGE image_final_render;
    DistortionEffect_t effect;
    Timer timer;
    int id = 0;
    EngineController controller;
} Engine;


void Engine_Init(Engine* engine, ResourceManager* rm, const char* name);

void Engine_AddId(Engine* engine, int id);
// 引擎绑定更新函数
void Engine_Attachment(Engine* engine, Entity* entity, Vector2f local_offset, float angle_offset);

void Engine_Update(Engine* engine);

void Engine_Advance(Engine* engine);

void Engine_Stop(Engine* engine);



#endif //STARSECTOR_NANO_ENGINE_H
