#include <iostream>
#include "ExplosionAPI.h"
#include "Distortion.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_THREADS 4
#define MAX_EXPLOSIONS 32

ExplosionAPI* ExplosionAPI_CreateExplosion(ResourceManager* rsm,
                                           RenderManager* rdm,
                                           ExplosionType type,
                                           Vector2f pos,
                                           float duration) {
    ExplosionAPI* exp = new(ExplosionAPI);
    exp->m_type = type;
    exp->m_position = pos;
    exp->m_duration = duration;
    ResourceManager_GetIMAGE(rsm, &exp->empty_image, "explosion", "empty");

    // 根据不同类型加载不同的资源
    switch (type) {
        case EXPLOSION_FIRE: {
            ResourceManager_GetIMAGE(rsm, &exp->m_flashImg, "explosion", "flash_fire1");
            ResourceManager_GetIMAGE(rsm, &exp->m_smokeImg, "explosion", "empty");
            exp->m_scale = 0.5f;
            break;
        }

        case EXPLOSION_LASER_FIRE:
            ResourceManager_GetIMAGE(rsm, &exp->m_flashImg, "explosion", "flash_fire2");
            ResourceManager_GetIMAGE(rsm, &exp->m_smokeImg, "explosion", "flash_fire2");
            exp->m_scale = 0.5f;
            break;

        case EXPLOSION_SHIP: {
            // 飞船爆炸特效
            ResourceManager_GetIMAGE(rsm, &exp->m_flashImg, "explosion", "ship_boom_light_flash");
            ResourceManager_GetIMAGE(rsm, &exp->m_smokeImg, "explosion", "ship_boom_light");
            exp->m_scale = 2.5f;
            break;
        }

        case EXPLOSION_HEAVY_ONHIT: {
            ResourceManager_GetIMAGE(rsm, &exp->m_flashImg, "explosion", "flash");
            int randomSmokeIndex1 = rand() % 3 + 1;
            char smokeKey1[32];
            sprintf(smokeKey1, "smoke_heavy%d", randomSmokeIndex1);
            ResourceManager_GetIMAGE(rsm, &exp->m_smokeImg, "explosion", smokeKey1);
            exp->m_scale = 1;
            break;
        }

        case EXPLOSION_LASER_HIT:
            ResourceManager_GetIMAGE(rsm, &exp->m_flashImg, "explosion", "flash_fire2");
            ResourceManager_GetIMAGE(rsm, &exp->m_smokeImg, "explosion", "flash_fire2");
            exp->m_scale = 0.5f;
            break;

        case EXPLOSION_SHELL:
        default: {
            ResourceManager_GetIMAGE(rsm, &exp->m_flashImg, "explosion", "flash");
            int randomSmokeIndex = rand() % 6 + 1;
            char smokeKey[32];
            sprintf(smokeKey, "smoke%d", randomSmokeIndex);
            ResourceManager_GetIMAGE(rsm, &exp->m_smokeImg, "explosion", smokeKey);
            exp->m_scale = 0.4f;
            break;
        }
    }

    exp->flashImg_after_render = exp->m_flashImg;
    exp->smokeImg_after_render = exp->m_smokeImg;
    exp->image_after_render = exp->empty_image;

    exp->smoke_alpha = 1.0f;
    exp->flash_alpha = 1.0f;
    exp->m_elapsed = 0.0f;
    exp->m_blurRadius = 0.0f;
    exp->is_active = 1;

    pthread_mutex_init(&exp->mutex, NULL);

    exp->id = RenderManager_AddEffect(rdm,
                                      &exp->image_after_render,
                                      pos.x,
                                      pos.y,
                                      0);
    return exp;
}

void ExplosionAPI_ResetExplosion(ResourceManager* rsm,
                                 ExplosionAPI* exp,
                                 int id,
                                 ExplosionType type,
                                 Vector2f pos,
                                 float duration) {
    pthread_mutex_destroy(&exp->mutex);

    exp->m_type = type;
    exp->m_position = pos;
    exp->m_duration = duration;
    ResourceManager_GetIMAGE(rsm, &exp->empty_image, "explosion", "empty");

    // 同样的类型分支处理
    switch (type) {
        case EXPLOSION_FIRE:
            ResourceManager_GetIMAGE(rsm, &exp->m_flashImg, "explosion", "flash_fire1");
            ResourceManager_GetIMAGE(rsm, &exp->m_smokeImg, "explosion", "flash_fire1");
            exp->m_scale = 0.5f;
            break;

        case EXPLOSION_LASER_FIRE:
            ResourceManager_GetIMAGE(rsm, &exp->m_flashImg, "explosion", "flash_fire2");
            ResourceManager_GetIMAGE(rsm, &exp->m_smokeImg,  "explosion", "flash_fire2");
            exp->m_scale = 0.5f;
            break;

        case EXPLOSION_LASER_HIT:
            ResourceManager_GetIMAGE(rsm, &exp->m_flashImg, "explosion", "flash_fire2");
            ResourceManager_GetIMAGE(rsm, &exp->m_smokeImg, "explosion", "flash_fire2");
            exp->m_scale = 0.5f;
            break;

        case EXPLOSION_SHIP:
            // 飞船爆炸特效
            ResourceManager_GetIMAGE(rsm, &exp->m_flashImg, "explosion", "ship_boom_light_flash");
            ResourceManager_GetIMAGE(rsm, &exp->m_smokeImg, "explosion", "ship_boom_light");
            exp->m_scale = 2.5f;
            break;

        case EXPLOSION_HEAVY_ONHIT: {
            ResourceManager_GetIMAGE(rsm, &exp->m_flashImg, "explosion", "flash");
            int randomSmokeIndex1 = rand() % 3 + 1;
            char smokeKey1[32];
            sprintf(smokeKey1, "smoke_heavy%d", randomSmokeIndex1);
            ResourceManager_GetIMAGE(rsm, &exp->m_smokeImg, "explosion", smokeKey1);
            exp->m_scale = 1;
            break;
        }


        case EXPLOSION_SHELL:
        default:
            ResourceManager_GetIMAGE(rsm, &exp->m_flashImg, "explosion", "flash");

            int randomSmokeIndex = rand() % 6 + 1;
            char smokeKey[32];
            sprintf(smokeKey, "smoke%d", randomSmokeIndex);
            ResourceManager_GetIMAGE(rsm, &exp->m_smokeImg, "explosion", smokeKey);

            exp->m_scale = 0.4f;
            break;
    }

    exp->flashImg_after_render = exp->m_flashImg;
    exp->smokeImg_after_render = exp->m_smokeImg;
    exp->image_after_render = exp->empty_image;

    exp->smoke_alpha = 1.0f;
    exp->flash_alpha = 1.0f;
    exp->m_elapsed = 0.0f;
    exp->m_blurRadius = 0.0f;
    exp->is_active = 1;

    pthread_mutex_init(&exp->mutex, NULL);

    exp->id = id;
}

bool ExplosionAPI_UpdateExplosion(ExplosionAPI* exp, float deltaTime) {
    if(exp->smoke_alpha < 0 && exp->flash_alpha < 0){
        return false;
    }
    switch(exp->m_type){
        case EXPLOSION_FIRE:
            exp->flash_alpha -= 0.06f;
            exp->smoke_alpha -= 0.06f;
            break;
        case EXPLOSION_LASER_FIRE:
            exp->flash_alpha -= 0.2f;
            exp->smoke_alpha -= 0.2f;
            break;
        case EXPLOSION_LASER_HIT:
            exp->flash_alpha -= 0.15f;
            exp->smoke_alpha -= 0.15f;
            break;
        case EXPLOSION_SHIP:
            exp->flash_alpha -= 0.02f;
            exp->smoke_alpha -= 0.01f;
            break;
        case EXPLOSION_SHELL:
            exp->flash_alpha -= 0.03f;
            exp->smoke_alpha -= 0.01f;
            break;
        default:
            exp->flash_alpha -= 0.03f;
            exp->smoke_alpha -= 0.01f;
            break;
    }

    ImageProcessing_AdjustImageTransparency(&exp->smokeImg_after_render,
                                            &exp->m_smokeImg,
                                            exp->smoke_alpha);
    ImageProcessing_AdjustImageTransparency(&exp->flashImg_after_render,
                                            &exp->m_flashImg,
                                            exp->flash_alpha);

    // 计算生命周期进度(0到1)
    float progress = 1.0f - (exp->smoke_alpha / 1.0f);
    progress = fmaxf(0.0f, fminf(1.0f, progress));
    exp->m_blurRadius = exp->m_maxBlurRadius * progress;

    // 无限膨胀的指数缩放控制
    const float k = 2.0f;
    const float initialScale = exp->m_scale;
    const float initialGrowth = 0.8f;

    // 基础指数增长 + 后期线性无限增长
    float currentScale = initialScale +
                         initialGrowth * (1.0f - expf(-k * progress));

    Distortion_ZoomImage(&exp->smokeImg_after_render_zoom,
                         &exp->smokeImg_after_render,
                         currentScale,
                         true);

    Distortion_ZoomImage(&exp->flashImg_after_render_zoom,
                         &exp->flashImg_after_render,
                         currentScale,
                         true);

    ImageProcessing_FastKawaseBlur(&exp->smokeImg_after_render, (exp->m_blurRadius / 2), true);

    ImageProcessing_BlendImages(&exp->image_after_render,
                                &exp->flashImg_after_render_zoom,
                                &exp->smokeImg_after_render_zoom);

    return true;
}

void ExplosionAPI_RenderExplosion(ExplosionAPI* exp, RenderManager* rdm) {
    // 锁定当前爆炸对象
    pthread_mutex_lock(&exp->mutex);

    RenderManager_Update(rdm,
                         exp->id,
                         exp->m_position.x,
                         exp->m_position.y,
                         0,
                         &exp->image_after_render);

    // 解锁
    pthread_mutex_unlock(&exp->mutex);
}

void ExplosionAPI_ClearExplosion(ExplosionAPI* exp, RenderManager* rdm) {
    pthread_mutex_lock(&exp->mutex);
    RenderManager_DeleteNode(rdm, exp->id);
    pthread_mutex_unlock(&exp->mutex);

    pthread_mutex_destroy(&exp->mutex);
    delete exp;
}

void ExplosionAPI_ClearExplosion(ExplosionAPI* exp){
    exp->m_position = {0, 0};
    exp->smoke_alpha = 0;
    exp->m_elapsed = 0;
    exp->m_scale = 0;
    exp->m_duration = 0;
}
