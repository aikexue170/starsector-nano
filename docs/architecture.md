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
    
    // 获取玩家位置
    position = player->entity_data.position;
    
    // 更新摄像机位置
    Camera_HandleMouseMessage(&camera, &msg, deltaTime);
    RenderManager_UpdateCameraScreenPosition(&camera);
    
    // 对象池更新
    ShipPool_Update(&rsm, &rdm, &explosionSeqPool, ship_pool, &camera, deltaTime);
    BulletPool_Update(bullet_pool, &rdm, deltaTime);
    ExplosionPool_UpdateMT(explosion_pool, deltaTime);
    CombatDataPool_Update(combat_data_pool, ship_pool);
    
    // 更新序列爆炸
    for (int i = 0; i < MAX_SEQUENCED_EXPLOSIONS; i++) {
        if (explosionSeqPool.inUse[i] && explosionSeqPool.sequences[i].isActive) {
            ExplosionSequence_Update(&explosionSeqPool.sequences[i], explosion_pool, &rsm, &rdm, deltaTime);
            
            // 如果序列播放完毕，释放回对象池
            if (!explosionSeqPool.sequences[i].isActive) {
                SequencedExplosionPool_Release(&explosionSeqPool, &explosionSeqPool.sequences[i]);
            }
        }
    }
    
    // 更新所有AI飞船
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (ship_pool->inUse[i] && !ship_pool->ships[i]->isPlayer) {
            FSM_Update(ship_pool->ships[i], &aiCtx, combat_data_pool, &fsmRes);
        }
    }
    
    // 键盘控制检测和渲染
    // ...
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
name,id_name,designation,mass,hitpoint,acceleration,rotational_acceleration,image_center_offset_x,image_center_offset_y
传输门,PRI_TransmissionGate,采矿母舰,1000,200000,50,10,162,360
环探针,RUI_RingProbe,神秘 战列舰,1000,200000,50,10,162,360
```

### 碰撞形状配置
多边形碰撞形状通过JSON定义：

```json
// assets/data/ship/PRI_TransmissionGate.json
{
  "vertices": [
    {"x": 155, "y": 145},
    {"x": 61, "y": 404},
    {"x": 57, "y": 466},
    {"x": 87, "y": 560},
    {"x": 243, "y": 559},
    {"x": 271, "y": 465},
    {"x": 263, "y": 400},
    {"x": 178, "y": 153}
  ],
  "engines": [
    {
      "name": "PRI_engine",
      "angle": -90,
      "attachment_point": {"x": 0, "y": 220},
      "attachment_angle_offset": 0
    }
  ],
  "weapons": [
    {
      "name": "Proton",
      "angle": -180,
      "attachment_point": {"x": 163, "y": 250},
      "offset": {"x": 0, "y": 0}
    }
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
    IMAGE image_with_light;          // 带光效的图像
    HitPolygon* polygon;             // JSON读取的碰撞多边形
    Vector2f position;               // 实时更新的位置
    Vector2f velocity;               // 实时更新的速度
    Vector2f image_center_offset;    // CSV读取的图像中心偏移
    float rotational_velocity;       // 实时更新的角速度
    float moment_of_inertia;         // 转动惯量
    float acceleration;              // CSV读取的加速度
    float rotational_acceleration;   // CSV读取的旋转加速度
    float angle;                     // 实时更新的角度
    int hitpoint;                    // CSV读取的生命值
    float mass;                      // CSV读取的质量
    int id;                          // 渲染器分配的ID
    Vector2f* transformed_vertices;  // 实时更新的变换后顶点数组
    const char* id_name;             // 实体标识名
    LightController light_controller;// 光效控制器
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
    FSM_STATE_PATROL,  // 巡逻状态，保持位置缓慢旋转
    FSM_STATE_ATTACK,  // 攻击状态，接近目标并射击
    FSM_STATE_EVADE    // 规避状态，躲避攻击
} FSMState;
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