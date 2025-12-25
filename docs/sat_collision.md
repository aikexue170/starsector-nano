# SAT碰撞检测：怎么知道两个形状撞上了

## 🎯 问题：怎么判断两个多边形是否相交

想象两个任意形状的飞船在太空中飞，怎么知道它们撞上了？

**简单方法不行**：
- ❌ 检查点是否在多边形内：太慢
- ❌ 检查边界框：不精确（会有很多"误报"）
- ❌ 像素级检测：更慢

**SAT算法**：又快又准！

## 📐 SAT算法思想：找一条"分离轴"

### 核心思想
如果两个凸多边形**没有**碰撞，那么一定存在一条直线，能把它们完全分开。

这条直线叫做"分离轴"。我们只需要检查多边形的每条边的法线方向，看看能不能找到这样的分离轴。

![SAT示意图](https://upload.wikimedia.org/wikipedia/commons/thumb/5/5e/Separating_axis_theorem.svg/400px-Separating_axis_theorem.svg.png)

### 简单比喻
就像检查两个投影有没有重叠：
1. 把两个多边形"投影"到一条直线上
2. 如果投影不重叠，说明没撞上
3. 如果所有方向的投影都重叠，说明撞上了

## 🔧 实现步骤：四步搞定

### 第一步：准备数据
每个多边形由顶点数组定义：

```c
// 三角形飞船的顶点（局部坐标）
Vector2f ship_vertices[] = {
    {0, -50},    // 顶部
    {-40, 40},   // 左下
    {40, 40}     // 右下
};
```

### 第二步：坐标变换
飞船在飞、在转，所以顶点位置要实时计算。实际项目中使用了Polygon_GetTransformedVertices函数：

```c
// src/tool/Polygon.cpp
void Polygon_GetTransformedVertices(const HitPolygon* polygon, 
                                   const Vector2f* position, 
                                   Vector2f image_center_offset, 
                                   float angle, 
                                   Vector2f* result) {
    for (int i = 0; i < polygon->vertex_count; i++) {
        Vector2f rotated;
        Vector2f centered_vertex;
        Vector2f_subtract(&polygon->vertices[i], &image_center_offset, &centered_vertex);
        Vector2f_rotate(&centered_vertex, angle, &rotated);
        Vector2f_add(&rotated, position, &result[i]);
    }
}
```

### 第三步：检查分离轴
对每个多边形的每条边，检查它的法线方向。实际代码使用内部静态函数：

```c
// src/tool/Polygon.cpp (内部静态函数)
static void get_axes(const Vector2f* vertices, int count, Vector2f* axes) {
    for (int i = 0; i < count; i++) {
        Vector2f current = vertices[i];
        Vector2f next = vertices[(i+1)%count];
        Vector2f edge = {next.x - current.x, next.y - current.y};
        Vector2f normal = {-edge.y, edge.x};
        Vector2f_normalize(&normal);
        axes[i] = normal;
    }
}

static void project_polygon(const Vector2f* vertices, int count,
                            const Vector2f* axis,
                            float* min, float* max) {
    *min = INFINITY;
    *max = -INFINITY;
    for (int i = 0; i < count; i++) {
        float proj = Vector2f_dot(&vertices[i], axis);
        *min = fminf(*min, proj);
        *max = fmaxf(*max, proj);
    }
}
```

### 第四步：检查所有轴
实际项目中SAT算法实现在Polygon_CollisionSAT函数中：

```c
// src/tool/Polygon.cpp
bool Polygon_CollisionSAT(const HitPolygon* a, const HitPolygon* b, 
                         Vector2f* a_verts, Vector2f* b_verts,
                         Vector2f* collision_point, Vector2f* separation_vector) {
    const int a_count = a->vertex_count;
    const int b_count = b->vertex_count;

    // 生成分离轴
    Vector2f axes[64];
    int axis_count = 0;

    // 获取多边形A的轴
    get_axes(a_verts, a_count, axes);
    axis_count += a_count;

    // 获取多边形B的轴
    get_axes(b_verts, b_count, axes + axis_count);
    axis_count += b_count;

    float min_overlap = INFINITY;
    Vector2f min_axis = {0};
    Vector2f centroid_a = calculate_centroid(a_verts, a_count);
    Vector2f centroid_b = calculate_centroid(b_verts, b_count);

    // 检测所有分离轴
    for (int i = 0; i < axis_count; i++) {
        Vector2f axis = axes[i];
        Vector2f_normalize(&axis);

        // 投影计算
        float a_min, a_max, b_min, b_max;
        project_polygon(a_verts, a_count, &axis, &a_min, &a_max);
        project_polygon(b_verts, b_count, &axis, &b_min, &b_max);

        // 分离轴检测
        if (a_max < b_min || b_max < a_min) {
            return false;
        }

        // 计算重叠量
        float overlap = fminf(a_max - b_min, b_max - a_min);
        if (overlap < min_overlap) {
            min_overlap = overlap;
            min_axis = axis;

            // 确定分离方向
            Vector2f delta_centroid = {centroid_b.x - centroid_a.x, centroid_b.y - centroid_a.y};
            if (Vector2f_dot(&delta_centroid, &axis) < 0) {
                min_axis.x = -min_axis.x;
                min_axis.y = -min_axis.y;
            }
        }
    }

    // 计算碰撞结果
    if (collision_point) {
        collision_point->x = (centroid_a.x + centroid_b.x) * 0.5f;
        collision_point->y = (centroid_a.y + centroid_b.y) * 0.5f;
    }

    if (separation_vector) {
        separation_vector->x = min_axis.x * min_overlap;
        separation_vector->y = min_axis.y * min_overlap;
    }

    return true;
}
```

## 🎮 在游戏里怎么用

### 1. 定义碰撞形状
在JSON文件里定义多边形：

```json
{
  "vertices": [
    {"x": 0, "y": -50},
    {"x": -40, "y": 40},
    {"x": 40, "y": 40}
  ]
}
```

### 2. 加载和缓存
游戏启动时加载多边形，并缓存起来：

```c
HitPolygon* LoadPolygon(const char* filename) {
    // 先检查缓存
    if (在缓存中找到) {
        return 缓存中的多边形;
    }
    
    // 加载JSON文件
    // 解析顶点数据
    // 存入缓存
    
    return 多边形;
}
```

### 3. 每帧检测
```c
bool CheckCollision(Entity* ship1, Entity* ship2) {
    // SAT检测
    Vector2f collision_point, separation_vector;
    return Polygon_CollisionSAT(
        ship1->polygon, ship2->polygon,
        ship1->transformed_vertices, ship2->transformed_vertices,
        &collision_point, &separation_vector
    );
}
```

## 💡 优化技巧

### 1. 包围盒先筛
先检查两个物体的包围盒是否相交，如果不相交，直接返回false。

### 2. 缓存变换结果
如果物体没动，不用重新计算顶点位置。

### 3. 减少顶点数
碰撞多边形不用太精确，三角形或四边形就够了。

### 4. 空间分区
把屏幕分成网格，只检查相邻网格里的物体。

## 🎯 实际代码位置

项目里的SAT实现在：
- `include/Polygon.h`：多边形数据结构
- `src/tool/Polygon.cpp`：SAT算法实现
- `src/Entity.cpp`：碰撞检测调用

## 🔧 调试：按F1看碰撞框

游戏里按F1可以显示红色碰撞框，方便调试：

```c
// 在渲染代码里
if (show_collision_polygons) {
    setlinecolor(RGB(255, 0, 0));  // 红色
    for (int i = 0; i < polygon->vertex_count; i++) {
        int j = (i + 1) % polygon->vertex_count;
        line(verts[i].x, verts[i].y, verts[j].x, verts[j].y);
    }
}
```

## 📚 给大一学生的数学小课堂

### 需要知道的数学
1. **向量**：有方向和大小的量 `(x, y)`
2. **点积**：`a·b = ax*bx + ay*by`，用来计算投影
3. **法向量**：垂直于边的向量，`(-dy, dx)`
4. **三角函数**：`sin`和`cos`用来旋转

### 算法复杂度
- 最坏情况：O(n×m)，n和m是多边形边数
- 实际很快：因为多边形边数很少（3-8条边）

### 常见错误
1. **顶点顺序**：必须是顺时针或逆时针，不能乱序
2. **凸多边形**：SAT只适用于凸多边形（没有凹进去的部分）
3. **归一化**：法向量要归一化（变成单位长度）

## 🎯 总结：为什么用SAT

1. **精确**：能处理任意凸多边形
2. **快速**：比像素检测快得多
3. **信息丰富**：不仅能判断是否碰撞，还能知道碰撞深度和方向
4. **广泛应用**：几乎所有2D游戏都用这个或类似算法

这个算法是游戏物理的基础。理解它，你就理解了2D碰撞检测的核心。