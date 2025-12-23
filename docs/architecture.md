# 项目架构设计

## 系统概述

Starsector Nano是一个使用C语言和EasyX图形库开发的2D太空战斗游戏。项目采用模块化设计，核心系统包括实体管理、物理模拟、碰撞检测、渲染系统和AI行为控制。

## 核心架构

### 主循环结构
游戏运行在60FPS的主循环中，每帧执行以下操作：

```c
// src/main.cpp 主循环
while(!game_exit){
    deltaTime = DeltaTime_Get();
    
    // 输入处理
    Camera_HandleMouseMessage(&camera, &msg, deltaTime);
    
    // 游戏状态更新
    ShipPool_Update(&rsm, &rdm, &explosionSeqPool, ship_pool, &camera, deltaTime);
    BulletPool_Update(bullet_pool, &rdm, deltaTime);
    ExplosionPool_UpdateMT(explosion_pool, deltaTime);
    
    // 碰撞检测（在ShipPool_Update内部处理）
    // 渲染（在各Update函数中调用）
}
```

### 对象池系统

为提高性能，项目实现了三个对象池：

1. **ShipPool**：管理最多100艘飞船，包含实体数据、引擎系统和武器系统
2. **BulletPool**：管理子弹对象，自动回收超出范围或生命期结束的子弹
3. **ExplosionPool**：管理爆炸效果，使用多线程更新（4个工作线程）

## 数据驱动设计

### 资源配置系统
项目使用JSON注册表统一管理资源路径：

```json
// assets/data/register.json
{
  "entity": {
    "RUI_RingProbe": "assets/graphic/ship/RUI_RingProbe.png",
    "PRI_TransmissionGate": "assets/graphic/ship/PRI_TransmissionGate.png"
  },
  "engine": {
    "RUI_engine": "assets/graphic/fx/RUI_engine.png"
  }
}
```

### 实体属性配置
物理属性通过CSV文件配置：

```csv
# assets/data/ship/ship_data.csv
name,type,mass,hitpoint,acceleration,rotational_acceleration,image_center_x,image_center_y
PRI_TransmissionGate,ship,1000,500,200.0,180.0,162.0,356.0
RUI_RingProbe,ship,800,400,220.0,200.0,150.0,150.0
```

### 碰撞形状配置
多边形碰撞形状通过JSON定义：

```json
// assets/data/ship/PRI_TransmissionGate.json
{
  "vertices": [
    {"x": 0, "y": -50},
    {"x": -40, "y": 40},
    {"x": 40, "y": 40}
  ]
}
```

## 核心数据结构

### 实体（Entity）
所有游戏对象的基础结构：

```c
// include/Entity.h
typedef struct {
    IMAGE image;
    HitPolygon* polygon;
    Vector2f position;
    Vector2f velocity;
    Vector2f image_center_offset;
    float rotational_velocity;
    float moment_of_inertia;
    float acceleration;
    float rotational_acceleration;
    float angle;
    int hitpoint;
    float mass;
    int id;
    Vector2f* transformed_vertices;
    const char* id_name;
    LightController light_controller;
} Entity;
```

### 飞船（ShipAPI）
扩展实体，添加引擎和武器系统：

```c
// include/ShipAPI.h
typedef struct ShipAPI {
    Entity entity_data;
    Engine engine[5];
    int engine_count;
    WeaponAPI weapon[10];
    int weapon_count;
    bool isPlayer;
    int team;
    Vector2f aim_position;
} ShipAPI;
```

## 渲染系统

### 分层渲染架构
渲染管理器使用链表管理不同渲染层：

```c
// include/Manager.h
typedef struct RenderManager {
    RenderNode* backgrounds;
    RenderNode* entities;
    RenderNode* uiElements;
    RenderNode* weapons;
    RenderNode* effects;
    IdNode* idList;
    int shakeAmplitude;
    int isShaking;
} RenderManager;
```

### 渲染流程
1. 背景层：星空背景
2. 实体层：飞船、子弹等游戏对象
3. 特效层：爆炸、引擎火焰等效果
4. UI层：战术地图、调试信息

## AI系统

### 有限状态机设计
AI基于三种状态实现简单行为：

```c
// include/CustomFSM.h
typedef enum {
    STATE_IDLE,        // 空闲状态，保持位置缓慢旋转
    STATE_ATTACK,      // 攻击状态，接近目标并射击
    STATE_EVADE        // 规避状态，躲避攻击
} AIState;
```

### 决策逻辑
每帧AI根据以下因素决定行为：
- 目标距离和角度
- 自身生命值状态
- 威胁评估（子弹接近程度）
- 团队协作（简单敌我识别）

## 性能优化

### 对象池技术
预分配对象，避免运行时内存分配开销。

### 多线程处理
爆炸效果使用4线程线程池并行更新。

### 碰撞检测优化
- 包围盒快速剔除
- SAT算法精确检测
- 多边形数据缓存

### 渲染优化
- 批量绘制调用
- 图像数据缓存
- 减少状态切换

## 模块依赖关系

```
main.cpp
├── ShipPool
│   ├── ShipAPI
│   │   ├── Entity
│   │   ├── Engine
│   │   └── WeaponAPI
│   └── CustomFSM
├── BulletPool
│   └── BulletAPI
├── ExplosionPool
│   ├── ExplosionAPI
│   └── ThreadPool
├── Camera
├── RenderManager
│   ├── ResourceManager
│   └── ImageProcessing
├── TacticalMap
├── Background
└── Control
```

## 调试功能

游戏内置调试系统，支持以下功能：
- F1：显示碰撞多边形
- F3：显示AI状态信息
- F5：玩家无敌模式
- F6：一击必杀模式
- F7：武器无冷却
- F8：无限弹药

这个架构展示了如何在C语言项目中实现完整的游戏系统，包括物理模拟、碰撞检测、AI行为和特效渲染。