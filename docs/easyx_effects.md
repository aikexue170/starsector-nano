# EasyX特效实现

## 🎨 EasyX图形库
EasyX是一个为C/C++设计的简单图形库，特点是简单易用，适合教学和快速开发。

## 🔥 引擎火焰效果
项目中的引擎火焰效果主要通过图片变形和光效合成实现。

### 引擎数据结构
```c
// include/Engine.h
typedef struct Engine {
    Vector2f position;           // 世界坐标位置
    float angle;                 // 角度
    IMAGE image;                 // 原始火焰图片
    IMAGE image_after_render;    // 处理后的图片
    DistortionEffect_t effect;   // 扭曲特效
    Timer timer;                 // 计时器
} Engine;
```

### 光效合成
实体光效通过Entity_Light函数实现：

```c
// src/Entity.cpp
void Entity_Light(Entity* entity) {
    // 获取图像缓冲区
    DWORD* img_buffer = GetImageBuffer(&entity->image);
    DWORD* light_buffer = GetImageBuffer(&entity->light_controller.light_image);
    int width = entity->image.getwidth();
    int height = entity->image.getheight();
    
    // 创建带光效的图像（如果还没有）
    if (entity->image_with_light.getwidth() == 0) {
        entity->image_with_light = entity->image;
    }
    
    DWORD* result_buffer = GetImageBuffer(&entity->image_with_light);
    
    // 混合基础图像和光效图像
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;
            COLORREF base_color = img_buffer[index];
            COLORREF light_color = light_buffer[index];
            
            // 提取RGB分量
            int base_r = GetRValue(base_color);
            int base_g = GetGValue(base_color);
            int base_b = GetBValue(base_color);
            
            int light_r = GetRValue(light_color);
            int light_g = GetGValue(light_color);
            int light_b = GetBValue(light_color);
            
            // 根据强度混合
            float intensity = entity->light_controller.intensity;
            int result_r = base_r + (int)(light_r * intensity);
            int result_g = base_g + (int)(light_g * intensity);
            int result_b = base_b + (int)(light_b * intensity);
            
            // 限制在0-255范围
            result_r = result_r > 255 ? 255 : result_r;
            result_g = result_g > 255 ? 255 : result_g;
            result_b = result_b > 255 ? 255 : result_b;
            
            result_buffer[index] = RGB(result_r, result_g, result_b);
        }
    }
}
```

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
typedef struct ExplosionSequence {
    IMAGE frames[MAX_EXPLOSION_FRAMES];
    int frameCount;
    int currentFrame;
    float frameDuration;
    float timer;
    bool isActive;
    Vector2f position;
    float scale;
} ExplosionSequence;
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