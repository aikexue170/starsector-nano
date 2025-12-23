# EasyX特效：用老图形库做出酷炫效果

## 🎨 EasyX是什么？

EasyX是一个为C/C++设计的简单图形库，特点是：
- **简单易用**：比OpenGL/DirectX简单得多
- **Windows专用**：只能在Windows上用
- **适合教学**：很多学校教C语言时用它

虽然是个"上古"库，但配合一些技巧，也能做出不错的效果。

## 🔥 引擎火焰：怎么让火焰动起来

### 问题：EasyX没有粒子系统
Unity/Unreal有现成的粒子系统，EasyX没有。怎么办？

**解决方案**：用图片+变形模拟火焰！

### 实现方法：三步变形

#### 第一步：准备火焰图片
一张静态的火焰图片：

![火焰图片](assets/graphic/fx/PRI_engine.png)

#### 第二步：应用水波效果
让火焰图片像水波一样波动：

```c
void RippleEffect_Update(RippleEffect_t* effect) {
    // 1. 波幅传播（像石头扔进水里）
    for (每个像素点) {
        // 新波幅 = 周围四个点的平均波幅 - 旧波幅
        buf2[i] = ((上+下+左+右)/2) - buf2[i];
        buf2[i] -= buf2[i] >> 5;  // 慢慢衰减
    }
    
    // 2. 交换缓冲区
    swap(buf, buf2);
}
```

#### 第三步：根据波幅扭曲图片
```c
void RippleEffect_Render(RippleEffect_t* effect) {
    for (每个像素点(y, x)) {
        // 根据波幅计算源图像坐标
        short wave = effect->buf[y * width + x];
        float factor = 1024 - wave;  // 扭曲系数
        
        // 计算采样位置（向中心收缩）
        int srcX = ((x - 中心X) * factor / 1024) + 中心X;
        int srcY = ((y - 中心Y) * factor / 1024) + 中心Y;
        
        // 边界检查
        if (srcX < 0) srcX = 0;
        if (srcX >= width) srcX = width - 1;
        if (srcY < 0) srcY = 0;
        if (srcY >= height) srcY = height - 1;
        
        // 从源图像采样颜色
        目标像素 = 源图像[srcY * width + srcX];
    }
}
```

### 效果：火焰看起来在"跳动"

![火焰动图](https://media.giphy.com/media/3o7abAHdYvZdBNnGZq/giphy.gif)

## 🌊 扭曲效果：让图像"扭动"起来

### 正弦波扭曲
像旗帜飘动一样的效果：

```c
void DistortionEffect_Render_S(DistortionEffect_t* effect) {
    // 时间累积，让动画动起来
    effect->time += effect->speed;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 计算正弦波偏移
            float wave = sin(x * 频率 + 时间) * 幅度;
            
            // 计算采样位置（y坐标加上波动）
            int srcY = y + (int)wave;
            
            // 边界检查
            if (srcY < 0) srcY = 0;
            if (srcY >= height) srcY = height - 1;
            
            // 复制像素
            目标[y*width+x] = 源[srcY*width+x];
        }
    }
}
```

### 参数调节
- **幅度**：波有多高，越大扭曲越厉害
- **频率**：波有多密，越大波纹越多
- **速度**：波移动多快，越大动画越快

## 🚀 引擎系统集成

### 引擎数据结构
```c
typedef struct Engine {
    Vector2f position;           // 世界坐标位置
    float angle;                 // 角度
    IMAGE image;                 // 原始火焰图片
    IMAGE image_after_render;    // 处理后的图片
    DistortionEffect_t effect;   // 扭曲特效
    Timer timer;                 // 计时器
} Engine;
```

### 引擎绑定到飞船
引擎位置要跟着飞船动：

```c
void Engine_Attachment(Engine* engine, Entity* ship, 
                      Vector2f local_offset, float angle_offset) {
    // 1. 计算旋转后的偏移
    Vector2f rotated;
    Vector2f_rotate(&local_offset, ship->angle, &rotated);
    
    // 2. 计算世界坐标
    engine->position.x = ship->position.x + rotated.x;
    engine->position.y = ship->position.y + rotated.y;
    
    // 3. 计算角度
    engine->angle = ship->angle + angle_offset;
}
```

### 推进时触发特效
```c
void ShipAPI_Forward(ShipAPI* ship, float deltaTime) {
    // ... 物理计算 ...
    
    // 触发所有引擎特效
    for (int i = 0; i < ship->engine_count; i++) {
        Engine_Advance(&ship->engine[i]);
        
        // 还可以触发涟漪
        RippleEffect_Disturb(&ship->engine[i].effect, 
                            中心X, 中心Y, 20, 500);
    }
}
```

## 💥 爆炸效果：多张图片序列

### 序列动画原理
像翻书一样，快速显示一系列图片：

```c
typedef struct ExplosionSequence {
    IMAGE frames[10];     // 10张爆炸图片
    int frame_count;      // 总共几帧
    int current_frame;    // 当前显示第几帧
    float frame_time;     // 每帧显示时间
    float timer;          // 计时器
    bool isActive;        // 是否在播放
} ExplosionSequence;
```

### 播放动画
```c
void ExplosionSequence_Update(ExplosionSequence* seq, float deltaTime) {
    if (!seq->isActive) return;
    
    seq->timer += deltaTime;
    
    // 时间到了就切到下一帧
    if (seq->timer >= seq->frame_time) {
        seq->timer = 0;
        seq->current_frame++;
        
        // 播完了就停止
        if (seq->current_frame >= seq->frame_count) {
            seq->isActive = false;
            seq->current_frame = 0;
        }
    }
}
```

### 多线程优化
爆炸效果计算量大，用多线程：

```c
// 初始化4个线程
ExplosionThreadPool_Init(4);

// 多线程更新爆炸池
ExplosionPool_UpdateMT(explosion_pool, deltaTime);
```

## 🎨 光效合成：让飞船"发光"

### 问题：EasyX没有Shader
现代游戏用Shader做发光，EasyX没有。

**解决方案**：图片叠加+透明度混合

### 实现方法
1. **准备光效图片**：半透明的光晕图片
2. **调整亮度**：根据距离、状态调整
3. **叠加到原图**：像Photoshop图层叠加

```c
void ApplyLightEffect(IMAGE* base, IMAGE* light, float intensity) {
    // 强度范围：0.0（无光）到1.0（最亮）
    for (每个像素) {
        // 获取基础颜色和光效颜色
        COLORREF base_color = 基础图像素;
        COLORREF light_color = 光效图像素;
        
        // 提取RGB分量
        int base_r = GetRValue(base_color);
        int base_g = GetGValue(base_color);
        int base_b = GetBValue(base_color);
        
        int light_r = GetRValue(light_color);
        int light_g = GetGValue(light_color);
        int light_b = GetBValue(light_color);
        
        // 混合：基础色 + 光效色 × 强度
        int result_r = base_r + (int)(light_r * intensity);
        int result_g = base_g + (int)(light_g * intensity);
        int result_b = base_b + (int)(light_b * intensity);
        
        // 限制在0-255范围
        result_r = min(255, result_r);
        result_g = min(255, result_g);
        result_b = min(255, result_b);
        
        // 写回像素
        结果图像素 = RGB(result_r, result_g, result_b);
    }
}
```

## 🔧 性能优化技巧

### 1. 避免频繁创建/销毁图片
```c
// 不好：每帧都创建新图片
IMAGE temp;
loadimage(&temp, "file.png");

// 好：预加载，重复使用
static IMAGE cached_image;
if (!loaded) {
    loadimage(&cached_image, "file.png");
    loaded = true;
}
```

### 2. 减少图像复制
直接操作内存缓冲区：

```c
DWORD* img_buffer = GetImageBuffer(&image);
// 直接操作img_buffer数组，比用EasyX函数快
```

### 3. 使用静态变量缓存计算结果
```c
void RenderEngineFlame(Engine* engine) {
    static IMAGE cached_result;  // 静态缓存
    static bool needs_update = true;
    
    if (needs_update) {
        // 计算特效...
        needs_update = false;
    }
    
    // 使用缓存的结果
    putimage(engine->position.x, engine->position.y, &cached_result);
}
```

## 🎯 实际代码位置

项目里的特效实现在：
- `include/Distortion.h`：扭曲特效数据结构
- `src/effect/Distortion.cpp`：水波和扭曲算法
- `src/Engine.cpp`：引擎特效集成
- `src/effect/SequencedExplosion.cpp`：序列爆炸

## 💡 给大一学生的建议

### 从简单开始
1. **先学会显示图片**：`loadimage`, `putimage`
2. **再学图片处理**：`GetImageBuffer`, 直接操作像素
3. **最后做特效**：基于前两步

### 调试技巧
1. **保存中间结果**：把处理后的图片保存到文件看看
2. **简化问题**：先在小图片上测试算法
3. **逐步添加**：先做静态效果，再加动画

### 学习资源
1. **EasyX文档**：了解基本函数
2. **图像处理基础**：卷积、滤波、混合
3. **游戏特效原理**：粒子系统、Shader基础

## 🚀 总结：老库也能出新效果

这个项目证明了：
1. **算法比库重要**：好的算法能在简单库上做出复杂效果
2. **创意比技术重要**：水波算法本来不是为火焰设计的，但用在这里很合适
3. **优化是关键**：直接内存操作、多线程、缓存，让老库也能跑得快

虽然EasyX功能有限，但通过巧妙的算法和优化，依然能做出令人印象深刻的游戏效果。