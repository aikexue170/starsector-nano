#include "Engine.h"



void Engine_Init(Engine* engine, ResourceManager* rm, const char* name) {
    // 初始化涟漪效果
    DistortionEffect_Init(&engine->effect, rm, name);
    // 初始化计时器
    Timer_Init(&engine->timer); // 直接初始化 Engine 中的 timer 成员
    Timer_Start(&engine->timer); // 启动计时器

    engine->image = engine->effect.src_img;
    engine->image_after_render = engine->effect.dest_img;

    engine->controller.zoom_x = 1;
    engine->controller.zoom_y = 1;
}

void Engine_AddId(Engine* engine, int id){
    engine->id = id;
}

// 引擎绑定更新函数（需要每帧调用）
void Engine_Attachment(Engine* engine, Entity* entity, Vector2f local_offset, float angle_offset) {
    // 计算旋转后的局部偏移
    Vector2f rotated_offset;
    Vector2f_rotate(&local_offset, entity->angle, &rotated_offset);

    // 计算实际世界坐标位置
    Vector2f_add(&entity->position, &rotated_offset, &engine->position);

    // 计算实际角度（保持初始角度偏移）
    engine->angle = entity->angle + angle_offset;
}

void Engine_Update(Engine* engine) {
    DistortionEffect_Update(&engine->effect);
    DistortionEffect_Render_S(&engine->effect, &engine->image_after_render);
}

void Engine_Advance(Engine* engine) {
    Distortion_ZoomImage(&engine->image_final_render, &engine->image_after_render, engine->controller.zoom_x, true, engine->controller.zoom_y);
    if(engine->controller.zoom_x < 1){
        engine->controller.zoom_x += 0.01;
    }
    if(engine->controller.zoom_y < 1){
        engine->controller.zoom_y += 0.01;
    }
}

void Engine_Stop(Engine* engine){
    Distortion_ZoomImage(&engine->image_final_render, &engine->image_after_render, engine->controller.zoom_x, true, engine->controller.zoom_y);
    if(engine->controller.zoom_x > 0.8){
        engine->controller.zoom_x -= 0.01;
    }
    if(engine->controller.zoom_y > 0.3){
        engine->controller.zoom_y -= 0.01;
    }
}

