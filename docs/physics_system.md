# 物理系统实现

## 运动系统

### 线性运动
实体位置基于速度和时间更新：

```c
// src/Entity.cpp
void Entity_Update(Entity* entity, float deltaTime) {
    // 检查旋转速度是否异常大
    if (fabs(entity->rotational_velocity) > MAX_ROTATIONAL_VELOCITY) {
        return; // 如果旋转速度异常大，则直接返回
    }

    // 检查线性速度是否异常大
    float linear_speed = sqrtf(entity->velocity.x * entity->velocity.x + entity->velocity.y * entity->velocity.y);
    if (linear_speed > MAX_LINEAR_SPEED) {
        return; // 如果线性速度异常大，则直接返回
    }

    // 应用旋转阻尼（按帧率标准化）
    const float angular_drag = powf(0.95f, deltaTime * 30.0f);
    entity->rotational_velocity *= angular_drag;
    // 应用线性阻尼（按帧率标准化）
    const float linear_drag = powf(0.95f, deltaTime * 10.0f);
    entity->velocity.x *= linear_drag;
    entity->velocity.y *= linear_drag;

    // 更新位置
    Vector2f velocity_delta;
    Vector2f_scale(&entity->velocity, deltaTime, &velocity_delta);
    Vector2f_add(&entity->position, &velocity_delta, &entity->position);

    // 更新角度
    entity->angle += entity->rotational_velocity * deltaTime;

    // 应用光效
    Entity_Light(entity);

    // 更新变换后的顶点
    Polygon_GetTransformedVertices(
            entity->polygon,
            &entity->position,
            entity->image_center_offset,
            entity->angle,
            entity->transformed_vertices
    );
}
```

### 推进力计算
前进时根据飞船角度计算推力方向：

```c
void Entity_Forward(Entity* entity, float deltaTime) {
    // 将角度从角度制转换为弧度制
    float radians = (entity->angle - 90.0f) * (M_PI / 180.0f);

    // 计算加速度在 X 和 Y 方向的分量
    float acceleration_x = entity->acceleration * cosf(radians);
    float acceleration_y = entity->acceleration * sinf(radians);

    // 更新速度
    entity->velocity.x += acceleration_x * deltaTime;
    entity->velocity.y += acceleration_y * deltaTime;
}

void Entity_Backward(Entity* entity, float deltaTime) {
    // 将角度从角度制转换为弧度制
    float radians = (entity->angle - 90.0f) * (M_PI / 180.0f);

    // 计算加速度在 X 和 Y 方向的分量（反向）
    float acceleration_x = -entity->acceleration * cosf(radians);
    float acceleration_y = -entity->acceleration * sinf(radians);

    // 更新速度
    entity->velocity.x += acceleration_x * deltaTime;
    entity->velocity.y += acceleration_y * deltaTime;
}
```

### 旋转运动
角速度控制飞船转向：

```c
void Entity_RotateLeft(Entity* entity, float deltaTime) {
    // 左转时减少角度（顺时针旋转）
    entity->rotational_velocity = -entity->rotational_acceleration;
}

void Entity_RotateRight(Entity* entity, float deltaTime) {
    // 右转时增加角度（逆时针旋转）
    entity->rotational_velocity = entity->rotational_acceleration;
}
```

## 碰撞检测

### SAT精确检测
使用SAT算法进行多边形碰撞检测（详见sat_collision.md）。

### 碰撞响应
碰撞检测和响应在Entity_Collision函数中实现：

```c
// src/Entity.cpp
bool Entity_Collision(Entity* entity, Entity* otherEntity, float deltaTime) {
    Vector2f collision_point;
    Vector2f separation_vector;
    bool is_colliding = Polygon_CollisionSAT(
            entity->polygon,
            otherEntity->polygon,
            entity->transformed_vertices,
            otherEntity->transformed_vertices,
            &collision_point,
            &separation_vector
    );

    if (is_colliding) {
        // 1. 统一使用物理坐标系（Y轴向上）
        Vector2f physics_separation = separation_vector;
        physics_separation.y *= -1; // 反转Y轴

        // 2. 冲量计算
        Vector2f separation_dir = physics_separation;
        Vector2f_normalize(&separation_dir);

        float total_mass = entity->mass + otherEntity->mass;
        float epsilon = 0.01;
        float e = 0.8f; // 恢复系数

        // 速度投影（已在物理坐标系）
        float v1_sep = Vector2f_dot(&entity->velocity, &separation_dir);
        float v2_sep = Vector2f_dot(&otherEntity->velocity, &separation_dir);
        float v_rel = v1_sep - v2_sep;

        float j = -(1 + e) * v_rel / (1/entity->mass + 1/otherEntity->mass + epsilon);

        Vector2f impulse;
        Vector2f_scale(&separation_dir, j, &impulse);

        // 3. 应用线速度变化
        Vector2f delta_v1, delta_v2;
        Vector2f_scale(&impulse, 1.0f/entity->mass, &delta_v1);
        Vector2f_scale(&impulse, -1.0f/otherEntity->mass, &delta_v2);

        Vector2f_add(&entity->velocity, &delta_v1, &entity->velocity);
        Vector2f_add(&otherEntity->velocity, &delta_v2, &otherEntity->velocity);

        // 4. 角速度计算
        // 获取物理坐标系的中心点
        Vector2f entity_center = {
                entity->position.x + entity->image_center_offset.x,
                -(entity->position.y + entity->image_center_offset.y) // Y轴反转
        };
        Vector2f other_center = {
                otherEntity->position.x + otherEntity->image_center_offset.x,
                -(otherEntity->position.y + otherEntity->image_center_offset.y) // Y轴反转
        };

        // 碰撞点转换到物理坐标系
        Vector2f physics_collision_point = {collision_point.x, -collision_point.y};

        // 计算半径向量（物理坐标系）
        Vector2f r_entity, r_other;
        Vector2f_subtract(&physics_collision_point, &entity_center, &r_entity);
        Vector2f_subtract(&physics_collision_point, &other_center, &r_other);

        // 物理坐标系下的冲量
        Vector2f physics_impulse = impulse;

        // 计算力矩（标准叉积公式 τ = r × F）
        float cross_entity = r_entity.x * physics_impulse.y - r_entity.y * physics_impulse.x;
        float cross_other = r_other.x * physics_impulse.y - r_other.y * physics_impulse.x;

        // 更新角速度
        entity->rotational_velocity -= 50* cross_entity / (entity->moment_of_inertia + epsilon);
        otherEntity->rotational_velocity += 50* cross_other / (otherEntity->moment_of_inertia + epsilon);

        // 5. 位置修正
        float position_correction = 0.2f;
        Vector2f correction;
        Vector2f_scale(&separation_vector, position_correction, &correction);
        Vector2f_subtract(&entity->position, &correction, &entity->position);
        Vector2f_add(&otherEntity->position, &correction, &otherEntity->position);

        return true;
    }
    return false;
}
```

## 数据配置

### CSV数据解析
物理参数从CSV文件加载：

```c
// src/Entity.cpp
void Entity_Create_FromCSV(
        ResourceManager* rm,
        Entity* entity,
        HitPolygon* polygon,
        Vector2f position,
        float angle,
        Vector2f velocity,
        const char* id_name,
        ResourceType resourceType
) {
    char filepath[256];
    switch(resourceType) {
        case SHIP:
            snprintf(filepath, sizeof(filepath), "assets/data/ship/ship_data.csv", id_name);
            break;
        case BULLET:
            snprintf(filepath, sizeof(filepath), "assets/data/bullet/bullet_data.csv", id_name);
            break;
        default:
            snprintf(filepath, sizeof(filepath), "assets/data/ship/ship_data.csv", id_name);
            break;
    }

    FILE* file = fopen(filepath, "r");
    char line[1024];
    int found = 0;
    int mass = 0;
    int hitpoint = 0;
    float acceleration = 0.0f;
    float rotational_acceleration = 0.0f;
    Vector2f image_center_offset = {0.0f, 0.0f};

    // 逐行解析CSV
    while (fgets(line, sizeof(line), file)) {
        char* columns[16];
        int column_count = 0;
        char* token = strtok(line, ",\n");

        while (token && column_count < 16) {
            columns[column_count++] = token;
            token = strtok(NULL, ",\n");
        }

        // 检查列数并匹配id_name
        if (column_count >= 9 && strcmp(columns[1], id_name) == 0) {
            mass = atoi(columns[3]);
            hitpoint = atoi(columns[4]);
            acceleration = (float)atof(columns[5]);
            rotational_acceleration = (float)atof(columns[6]);
            image_center_offset.x = (float)atof(columns[7]);
            image_center_offset.y = (float)atof(columns[8]);
            found = 1;
            break;
        }
    }
    fclose(file);

    if (!found) {
        fprintf(stderr, "Error: Entity '%s' not found in CSV\n", id_name);
    }

    // 初始化基础属性
    entity->polygon = polygon;
    entity->position = position;
    entity->velocity = velocity;
    entity->angle = angle;
    entity->rotational_velocity = 0.0f; // 初始角速度设为0
    entity->id = -1; // 等待渲染器分配
    ResourceManager_GetIMAGE(rm, &entity->image, "entity", id_name);
    entity->id_name = id_name;
    entity->light_controller.intensity = 0.5;
    ResourceManager_GetIMAGE(rm, &entity->light_controller.light_image, "light", id_name);
    
    // 填充CSV数据
    entity->mass = mass;
    entity->hitpoint = hitpoint;
    entity->acceleration = acceleration;
    entity->rotational_acceleration = rotational_acceleration;
    entity->image_center_offset = image_center_offset;
    
    // 计算转动惯量
    float width = image_center_offset.x*2;
    float height = image_center_offset.y*2;
    entity->moment_of_inertia = entity->mass * (width*width + height*height) / 12.0f;
    
    // 初始化变换后的顶点
    if (polygon && polygon->vertex_count > 0) {
        entity->transformed_vertices = (Vector2f*)malloc(polygon->vertex_count * sizeof(Vector2f));
        if (!entity->transformed_vertices) {
            fprintf(stderr, "Error: Failed to allocate transformed vertices\n");
        }
    }
}
```

## 物理模拟循环

### 碰撞检测流程
碰撞检测在ShipPool_Update函数中处理：

```c
// src/ShipPool.cpp
void ShipPool_Update(ResourceManager* rsm, RenderManager* rdm, SequencedExplosionPool* explosionSeqPool, ShipPool* pool, Camera* camera, float deltaTime) {
    // 更新所有飞船
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (pool->inUse[i]) {
            ShipAPI_Update(pool->ships[i], camera, deltaTime);
            
            // 碰撞检测（与其他飞船）
            for (int j = i + 1; j < MAX_SHIPS; j++) {
                if (pool->inUse[j]) {
                    if (Entity_Collision(&pool->ships[i]->entity_data, 
                                        &pool->ships[j]->entity_data, 
                                        deltaTime)) {
                        // 处理碰撞伤害
                        // ...
                    }
                }
            }
            
            // 碰撞检测（与子弹）
            // ...
        }
    }
}
```

## 调试功能

### 碰撞多边形显示
按F1键可以显示碰撞多边形：

```c
// src/Entity.cpp
void Entity_ShowPolygon(Entity* entity) {
    if (!entity->polygon || !entity->transformed_vertices) return;
    
    setlinecolor(RGB(255, 0, 0));  // 红色
    for (int i = 0; i < entity->polygon->vertex_count; i++) {
        int j = (i + 1) % entity->polygon->vertex_count;
        line((int)entity->transformed_vertices[i].x, 
             (int)entity->transformed_vertices[i].y,
             (int)entity->transformed_vertices[j].x, 
             (int)entity->transformed_vertices[j].y);
    }
}
```

这个物理系统实现了基本的牛顿力学模拟，包括线性运动、旋转运动、碰撞检测和响应，为游戏提供了真实的物理交互。