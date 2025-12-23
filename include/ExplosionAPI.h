#ifndef EXPLOSION_EFFECT_H
#define EXPLOSION_EFFECT_H

#include "Manager.h"
#include "ImageProcessing.h"
#include <pthread.h>

typedef enum {
    EXPLOSION_SHIP,     // 飞船爆炸（全屏白屏+震动）
    EXPLOSION_SHELL,     // 炮弹爆炸（光球+烟尘）
    EXPLOSION_FIRE,      // 开火烟尘+光效
    EXPLOSION_HEAVY_ONHIT,
    EXPLOSION_LASER_FIRE,
    EXPLOSION_LASER_HIT
} ExplosionType;

typedef struct Explosion {
    Vector2f    m_position;    // 爆炸中心坐标
    float       m_duration;    // 总持续时间(秒)
    float       m_elapsed;     // 已持续时间
    double      smoke_alpha;   // 当前透明度[0-1]
    double      flash_alpha;
    float       m_scale;       // 当前缩放比例
    ExplosionType m_type;      // 爆炸类型
    float       m_blurRadius;  // 当前模糊半径
    float       m_maxBlurRadius; // 最大模糊半径
    IMAGE       m_flashImg;    // 闪光贴图（资源引用）
    IMAGE       m_smokeImg;    // 烟尘贴图（资源引用）
    IMAGE       image_after_render;
    IMAGE       flashImg_after_render;
    IMAGE       smokeImg_after_render;
    IMAGE       smokeImg_after_render_zoom;
    IMAGE       flashImg_after_render_zoom;
    IMAGE       temp_image;    // 临时图像
    IMAGE       empty_image;
    int         id;
    int         is_active;     // 标记是否活跃
    pthread_mutex_t mutex;     // 互斥锁
} ExplosionAPI;

// 线程池初始化函数
void ExplosionThreadPool_Init(int max_threads);
void ExplosionThreadPool_Shutdown();

// 创建爆炸实例
ExplosionAPI* ExplosionAPI_CreateExplosion(ResourceManager* rsm,
                                           RenderManager* rdm,
                                           ExplosionType type,
                                           Vector2f pos,
                                           float duration);

void ExplosionAPI_ResetExplosion(ResourceManager* rsm,
                                 ExplosionAPI* exp,
                                 int id,
                                 ExplosionType type,
                                 Vector2f pos,
                                 float duration);

// 更新状态（返回是否存在）
bool ExplosionAPI_UpdateExplosion(ExplosionAPI* exp, float deltaTime);

// 更新渲染器中的exp对象
void ExplosionAPI_RenderExplosion(ExplosionAPI* exp, RenderManager* rdm);

// 清空实例（用于对象池回收）
void ExplosionAPI_ClearExplosion(ExplosionAPI* exp, RenderManager* rdm);

// 清空实例，但不销毁
void ExplosionAPI_ClearExplosion(ExplosionAPI* exp);
#endif // EXPLOSION_EFFECT_H