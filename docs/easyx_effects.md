# EasyX特效实现

## 🎨 EasyX图形库
EasyX是一个为C/C++设计的简单图形库，特点是简单易用，适合教学和快速开发。

## 🔥 引擎火焰效果
项目中的引擎火焰效果主要通过图片变形和光效合成实现。

### 引擎数据结构
```c
// include/Engine.h
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

typedef struct EngineController{
    float zoom_x;
    float zoom_y;
} EngineController;
```

### 光效合成
实体光效通过Entity_Light函数实现。光效图像通过`light_controller`结构体管理：

```c
// include/Entity.h
typedef struct LightController{
    IMAGE light_image;
    IMAGE light_image_after_adjust;
    float intensity;// 0 到 1
}LightController;
```

Entity结构体包含`light_controller`成员，用于存储光效图像和强度参数。光效合成过程将基础图像与光效图像混合，根据强度参数调整光效效果。

注意：具体的光效混合实现代码在`src/Entity.cpp`中的`Entity_Light`函数里，使用EasyX的图像缓冲区操作实现像素级混合。

## 💥 爆炸效果系统
爆炸效果使用对象池和多线程更新。

### 爆炸池数据结构
```c
// include/ExplosionPool.h
typedef struct ExplosionPool {
    ExplosionAPI explosions[MAX_EXPLOSIONS];
    bool inUse[MAX_EXPLOSIONS];
    int activeCount;
} ExplosionPool;
```

### 多线程更新
爆炸效果使用线程池并行更新：

```c
// src/ExplosionPool.cpp
void ExplosionPool_UpdateMT(ExplosionPool* pool, float deltaTime) {
    // 使用线程池并行更新所有活跃的爆炸效果
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (pool->inUse[i]) {
            // 将任务提交到线程池
            ThreadPool_SubmitTask(&pool->explosions[i], deltaTime);
        }
    }
}
```

### 序列爆炸
对于复杂的爆炸序列，使用SequencedExplosion：

```c
// include/SequencedExplosion.h
typedef struct {
    ExplosionSequenceConfig config;
    float timer;           // 计时器，控制爆炸间隔
    int remainingExplosions; // 剩余爆炸次数
    bool isActive;         // 是否正在播放
} ExplosionSequence;

typedef struct {
    Vector2f center;       // 爆炸中心点
    float radius;          // 爆炸随机范围半径
    int minExplosions;     // 最小爆炸次数
    int maxExplosions;     // 最大爆炸次数
    float minDelay;        // 两次爆炸间最小延迟（秒）
    float maxDelay;        // 两次爆炸间最大延迟（秒）
    float duration;        // 单次爆炸持续时间
    ExplosionType type;    // 爆炸类型
    bool isLooping;        // 是否循环播放
} ExplosionSequenceConfig;
```

## 🎨 图像处理效果
项目包含基本的图像处理功能。

### 扭曲效果
```c
// src/effect/Distortion.cpp
void DistortionEffect_Apply(DistortionEffect_t* effect, IMAGE* src, IMAGE* dst) {
    // 获取图像缓冲区
    DWORD* src_buffer = GetImageBuffer(src);
    DWORD* dst_buffer = GetImageBuffer(dst);
    int width = src->getwidth();
    int height = src->getheight();
    
    // 应用简单的正弦波扭曲
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 计算扭曲偏移
            float offset_x = sin(y * 0.1f + effect->time) * effect->amplitude;
            float offset_y = cos(x * 0.1f + effect->time) * effect->amplitude;
            
            int src_x = x + (int)offset_x;
            int src_y = y + (int)offset_y;
            
            // 边界检查
            if (src_x < 0) src_x = 0;
            if (src_x >= width) src_x = width - 1;
            if (src_y < 0) src_y = 0;
            if (src_y >= height) src_y = height - 1;
            
            // 复制像素
            dst_buffer[y * width + x] = src_buffer[src_y * width + src_x];
        }
    }
    
    effect->time += effect->speed;
}
```

## 🔧 性能优化

### 1. 对象池技术
所有游戏对象（飞船、子弹、爆炸）都使用对象池管理，避免频繁的内存分配和释放。

### 2. 图像缓存
处理后的图像会被缓存，避免每帧重新计算。

### 3. 多线程处理
爆炸效果使用线程池并行更新，提高性能。

## 🎯 实际代码位置

项目里的特效实现在：
- `include/Distortion.h`：扭曲特效数据结构
- `src/effect/Distortion.cpp`：扭曲算法
- `src/Engine.cpp`：引擎特效
- `src/effect/SequencedExplosion.cpp`：序列爆炸
- `src/Entity.cpp`：实体光效合成

## 💡 实现要点

1. **简单有效**：使用基本的图像处理技术实现视觉效果
2. **性能优先**：通过缓存和多线程优化性能
3. **易于扩展**：模块化设计，方便添加新特效

虽然EasyX功能有限，但通过巧妙的算法和优化，依然能实现不错的游戏效果。