# 开发指南：如何修改和扩展这个游戏

## 🛠️ 快速开始：改点简单的

### 改飞船速度
打开 `assets/data/ship/ship_data.csv`，找到你想改的飞船：

```csv
name,type,mass,hitpoint,acceleration,rotational_acceleration,image_center_x,image_center_y
PRI_TransmissionGate,ship,1000,500,200.0,180.0,162.0,356.0
```

- `acceleration`：加速度，越大加速越快
- `rotational_acceleration`：转向加速度，越大转向越快
- `mass`：质量，越大惯性越大
- `hitpoint`：生命值

改完保存，重新运行游戏就能看到效果。

### 改武器伤害
打开 `assets/data/weapon/weapon_data.csv`：

```csv
name,cooldown,range,energy_cost,projectile_count
Torrent,0.8,600,15,1
```

- `cooldown`：冷却时间，越小射速越快
- `range`：射程
- `projectile_count`：一次发射几发子弹

## 🚀 添加新飞船：三步搞定

### 第一步：准备图片
1. 找一张飞船图片（PNG格式，背景透明）
2. 放到 `assets/graphic/ship/` 文件夹
3. 名字用英文，比如 `my_ship.png`

### 第二步：配置数据
1. 在 `assets/data/ship/ship_data.csv` 最后加一行：

```csv
my_ship,ship,800,400,180.0,160.0,100.0,100.0
```

 2. 在 `assets/data/ship/` 创建 `my_ship.json`：

```json
{
  "vertices": [
    {"x": 0, "y": -50},
    {"x": -40, "y": 40},
    {"x": 40, "y": 40}
  ]
}
```

注意：实际项目中，引擎和武器的配置不是通过JSON文件，而是在代码中通过ShipAPI_AddEngine和ShipAPI_AddWeapon函数添加。

**说明**：
- `vertices`：碰撞多边形的顶点，画个三角形把飞船包住就行
- `engines`：引擎位置，`attachment_point`是相对于飞船中心的坐标
- `weapons`：武器位置，可以装多个

### 第三步：注册飞船
打开 `assets/data/register.json`，在 `entity` 部分添加：

```json
"entity": {
  "my_ship": "assets/graphic/ship/my_ship.png",
  ...其他飞船...
}
```

### 第四步：在游戏里使用
打开 `src/main.cpp`，在注册区添加：

```c
// 在注册区添加这行
ShipAPI* myShip = ShipPool_GetShip(ship_pool, &rsm, &rdm, "my_ship", 
                                  (Vector2f){500, 500}, 0, 
                                  (Vector2f){0, 0});
```

重新编译运行，你的新飞船就出现了！

## 🔫 添加新武器

### 第一步：准备武器图片
1. 武器图片放到 `assets/graphic/weapons/`
2. 子弹图片放到 `assets/graphic/bullet/`

### 第二步：配置武器
1. 在 `assets/data/weapon/weapon_data.csv` 添加：

```csv
my_weapon,1.2,800,20,3
```

 2. 在 `assets/data/weapon/` 创建 `my_weapon.JSON`：

```json
{
  "isLazer": false,
  "isBullet": true,
  "isMissile": false,
  "BulletName": "my_bullet"
}
```

3. 在 `assets/data/bullet/` 创建 `my_bullet.json`：

```json
{
  "damage": 50,
  "speed": 600,
  "lifetime": 2.0,
  "collision_radius": 8
}
```

### 第三步：注册资源
在 `register.json` 中添加：

```json
"weapon": {
  "my_weapon": "assets/graphic/weapons/my_weapon.png"
},
"entity": {
  "my_bullet": "assets/graphic/bullet/my_bullet.png"
}
```

### 第四步：给飞船装上
在你飞船的JSON配置里，把武器名字改成 `my_weapon`：

```json
"weapons": [
  {
    "name": "my_weapon",  // 改成你的新武器
    "angle": 0,
    "attachment_point": {"x": 20, "y": -20}
  }
]
```

## 🎨 修改视觉效果

### 改引擎火焰
引擎图片在 `assets/graphic/fx/` 文件夹。你可以：
1. 替换现有的 `PRI_engine.png` 或 `RUI_engine.png`
2. 或者创建新的引擎图片，然后在代码中通过ShipAPI_AddEngine函数添加时指定引擎类型

记得在 `register.json` 的 `engine` 部分注册新引擎图片。

### 改爆炸效果
爆炸图片在 `assets/graphic/fx/`，有很多张：
- `explosion1.png` 到 `explosion6.png`：小爆炸
- `explosion7.png` 到 `explosion9.png`：大爆炸
- `hit_glow.png`：命中闪光

你可以替换这些图片，或者添加新的。

## 🤖 修改AI行为

AI逻辑在 `src/CustomFSM.cpp`。主要修改三个函数：

### 1. 改攻击距离
在 `FSM_HandleAttackState` 函数里：

```c
// 大约第100行
float distance_to_target = Vector2f_Distance(&ship->entity_data.position, 
                                            &ctx->target_position);

// 如果距离大于500，就靠近
if (distance_to_target > 500.0f) {  // 把这个500改大或改小
    // 朝目标移动
}
```

### 2. 改开火频率
在同一个函数里：

```c
// 检查武器冷却
for (int i = 0; i < ship->weapon_count; i++) {
    if (WeaponAPI_IsReady(&ship->weapon[i])) {
        // 这里可以加条件，比如距离小于300才开火
        if (distance_to_target < 300.0f) {
            WeaponAPI_Fire(&ship->weapon[i], bullet_pool, &ship->entity_data);
        }
    }
}
```

### 3. 改状态转换条件
在 `FSM_DetermineNextState` 函数里：

```c
// 如果生命值低于30%，就逃跑
if (ship->entity_data.hitpoint < ship->entity_data.max_hitpoint * 0.3f) {
    return STATE_EVADE;
}

// 如果发现目标，就攻击
if (has_target && distance_to_target < 1000.0f) {  // 改这个1000
    return STATE_ATTACK;
}
```

## 🔧 编译和运行

### Windows + MinGW
```bash
# 在项目根目录
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
./starsector-nano.exe
```

### 用CLion（推荐）
1. 用CLion打开项目文件夹
2. 等它自动配置CMake
3. 点绿色三角运行

### 常见问题
**问题**：编译错误，找不到头文件
**解决**：确保在 `build` 文件夹里运行cmake

**问题**：运行时报错，找不到图片
**解决**：检查 `register.json` 里的路径是否正确

**问题**：游戏很卡
**解决**：减少飞船数量，或者在 `main.cpp` 里改 `MAX_SHIPS`

## 💡 调试技巧

### 显示调试信息
游戏内置了调试功能，按这些键：

- **F1**：显示碰撞框（红色线框）
- **F3**：显示AI状态（左上角文字）
- **F5**：玩家无敌
- **F6**：一击必杀
- **F7**：武器无冷却
- **F8**：无限弹药

### 加日志输出
在代码里加 `printf` 看变量值：

```c
printf("飞船位置: %.1f, %.1f\n", 
       ship->entity_data.position.x,
       ship->entity_data.position.y);
printf("AI状态: %d\n", ctx->current_state);
```

## 🚀 下一步可以做什么

### 简单修改
1. 改背景图片：替换 `assets/graphic/background/background1.jpg`
2. 改武器特效：替换子弹图片
3. 改游戏平衡：调整CSV文件里的数值

### 中等难度
1. 加新武器类型：比如导弹、激光
2. 改进AI：让敌人更聪明
3. 加新特效：新的爆炸效果

### 高级修改
1. 加多人游戏：需要网络编程
2. 加关卡系统：保存和加载游戏状态
3. 改进物理：更真实的碰撞

## 📚 学习资源

### 需要先学的
1. **C语言基础**：指针、结构体、函数
2. **EasyX图形库**：画图、处理鼠标键盘
3. **JSON和CSV**：数据格式

### 项目里的好代码
1. `src/main.cpp`：程序入口，看整体流程
2. `src/ShipAPI.cpp`：飞船的实现
3. `src/CustomFSM.cpp`：AI逻辑
4. `src/tool/Polygon.cpp`：碰撞检测

### 遇到问题怎么办
1. 先看错误信息
2. 在代码里加 `printf` 调试
3. 简化问题：先做个最小可运行例子
4. 问同学或老师

记住：这个项目最重要的是**实践**。别怕改坏，有Git可以回退。从改小地方开始，慢慢积累信心。