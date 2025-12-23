# 物理系统实现

## 运动系统

### 线性运动
实体位置基于速度和时间更新：

```c
// src/Entity.cpp
void Entity_Update(Entity* entity, float deltaTime) {
    // 限制最大速度
    float current_speed = Vector2f_Length(entity->velocity);
    if (current_speed > MAX_LINEAR_SPEED) {
        entity->velocity = Vector2f_Multiply(
            Vector2f_Normalize(entity->velocity),
            MAX_LINEAR_SPEED
        );
    }
    
    // 更新位置：s = s0 + v*t
    entity->position.x += entity->velocity.x * deltaTime;
    entity->position.y += entity->velocity.y * deltaTime;
    
    // 应用阻力（模拟太空微弱阻力）
    float drag_factor = 0.99f;
    entity->velocity.x *= drag_factor;
    entity->velocity.y *= drag_factor;
    
    // 清除微小速度
    if (fabs(entity->velocity.x) < 0.1f) entity->velocity.x = 0;
    if (fabs(entity->velocity.y) < 0.1f) entity->velocity.y = 0;
}
```

### 推进力计算
前进时根据飞船角度计算推力方向：

```c
void Entity_Forward(Entity* entity, float deltaTime) {
    float angle_rad = entity->angle * M_PI / 180.0f;
    Vector2f thrust_direction = {
        sin(angle_rad),
        -cos(angle_rad)  // 屏幕坐标系y轴向下
    };
    
    entity->velocity.x += thrust_direction.x * entity->acceleration * deltaTime;
    entity->velocity.y += thrust_direction.y * entity->acceleration * deltaTime;
}
```

### 旋转运动
角速度控制飞船转向：

```c
void Entity_RotateLeft(Entity* entity, float deltaTime) {
    entity->rotational_velocity -= entity->rotational_acceleration * deltaTime;
    
    if (entity->rotational_velocity < -MAX_ROTATIONAL_VELOCITY) {
        entity->rotational_velocity = -MAX_ROTATIONAL_VELOCITY;
    }
}

void Entity_UpdateRotation(Entity* entity, float deltaTime) {
    entity->angle += entity->rotational_velocity * deltaTime;
    
    // 规范化角度到[0, 360)
    while (entity->angle >= 360.0f) entity->angle -= 360.0f;
    while (entity->angle < 0.0f) entity->angle += 360.0f;
    
    // 旋转阻尼
    float rotational_drag = 0.95f;
    entity->rotational_velocity *= rotational_drag;
    
    if (fabs(entity->rotational_velocity) < 0.1f) {
        entity->rotational_velocity = 0;
    }
}
```

## 碰撞检测

### 包围盒快速检测
在进行精确碰撞检测前，先进行AABB检测：

```c
bool CheckBroadPhaseCollision(Entity* a, Entity* b) {
    float a_min_x = a->position.x - a->bounding_radius;
    float a_max_x = a->position.x + a->bounding_radius;
    float a_min_y = a->position.y - a->bounding_radius;
    float a_max_y = a->position.y + a->bounding_radius;
    
    float b_min_x = b->position.x - b->bounding_radius;
    float b_max_x = b->position.x + b->bounding_radius;
    float b_min_y = b->position.y - b->bounding_radius;
    float b_max_y = b->position.y + b->bounding_radius;
    
    return !(a_max_x < b_min_x || a_min_x > b_max_x ||
             a_max_y < b_min_y || a_min_y > b_max_y);
}
```

### SAT精确检测
如果包围盒相交，进行SAT多边形碰撞检测（详见sat_collision.md）。

## 碰撞响应

### 冲量计算
基于动量守恒的碰撞响应：

```c
void ResolveCollision(Entity* a, Entity* b, 
                     Vector2f collision_normal) {
    // 计算相对速度
    Vector2f relative_velocity = {
        b->velocity.x - a->velocity.x,
        b->velocity.y - a->velocity.y
    };
    
    // 计算沿碰撞法线方向的速度分量
    float velocity_along_normal = 
        relative_velocity.x * collision_normal.x +
        relative_velocity.y * collision_normal.y;
    
    // 如果物体正在分离，不处理碰撞
    if (velocity_along_normal > 0) {
        return;
    }
    
    // 计算恢复系数（半弹性碰撞）
    float restitution = 0.5f;
    
    // 计算冲量大小
    float j = -(1 + restitution) * velocity_along_normal;
    j /= (1 / a->mass) + (1 / b->mass);
    
    // 应用冲量到速度
    Vector2f impulse = {
        collision_normal.x * j,
        collision_normal.y * j
    };
    
    a->velocity.x -= impulse.x / a->mass;
    a->velocity.y -= impulse.y / a->mass;
    b->velocity.x += impulse.x / b->mass;
    b->velocity.y += impulse.y / b->mass;
}
```

### 旋转碰撞响应
考虑角速度的碰撞响应：

```c
void ResolveRotationalCollision(Entity* a, Entity* b,
                               Vector2f collision_point,
                               Vector2f collision_normal) {
    // 计算从质心到碰撞点的向量
    Vector2f ra = Vector2f_Subtract(collision_point, a->position);
    Vector2f rb = Vector2f_Subtract(collision_point, b->position);
    
    // 计算碰撞点的速度（包括线速度和角速度）
    Vector2f va = {
        a->velocity.x - a->rotational_velocity * ra.y,
        a->velocity.y + a->rotational_velocity * ra.x
    };
    
    Vector2f vb = {
        b->velocity.x - b->rotational_velocity * rb.y,
        b->velocity.y + b->rotational_velocity * rb.x
    };
    
    // 计算相对速度
    Vector2f relative_velocity = Vector2f_Subtract(vb, va);
    
    // 计算沿法线方向的速度
    float velocity_along_normal = Vector2f_Dot(relative_velocity, collision_normal);
    
    if (velocity_along_normal > 0) return;
    
    // 计算转动惯量项
    float ra_perp = ra.x * collision_normal.y - ra.y * collision_normal.x;
    float rb_perp = rb.x * collision_normal.y - rb.y * collision_normal.x;
    
    float denom = (1 / a->mass) + (1 / b->mass) +
                  (ra_perp * ra_perp) / a->moment_of_inertia +
                  (rb_perp * rb_perp) / b->moment_of_inertia;
    
    // 计算冲量
    float restitution = 0.5f;
    float j = -(1 + restitution) * velocity_along_normal / denom;
    
    // 应用冲量
    Vector2f impulse = Vector2f_Multiply(collision_normal, j);
    
    // 更新线速度
    a->velocity = Vector2f_Subtract(a->velocity, 
                                   Vector2f_Multiply(impulse, 1 / a->mass));
    b->velocity = Vector2f_Add(b->velocity, 
                               Vector2f_Multiply(impulse, 1 / b->mass));
    
    // 更新角速度
    a->rotational_velocity -= (ra.x * impulse.y - ra.y * impulse.x) / a->moment_of_inertia;
    b->rotational_velocity += (rb.x * impulse.y - rb.y * impulse.x) / b->moment_of_inertia;
}
```

### 伤害计算
基于动能的伤害模型：

```c
float CalculateCollisionDamage(Entity* a, Entity* b,
                              Vector2f collision_normal) {
    // 计算相对速度
    Vector2f relative_velocity = Vector2f_Subtract(b->velocity, a->velocity);
    float impact_speed = Vector2f_Length(relative_velocity);
    
    // 计算动能：E = 1/2 * m * v²
    float kinetic_energy_a = 0.5f * a->mass * impact_speed * impact_speed;
    float kinetic_energy_b = 0.5f * b->mass * impact_speed * impact_speed;
    
    // 总碰撞能量
    float total_energy = kinetic_energy_a + kinetic_energy_b;
    
    // 计算碰撞角度因子
    Vector2f a_direction = Vector2f_Normalize(a->velocity);
    Vector2f b_direction = Vector2f_Normalize(b->velocity);
    
    float dot_product = Vector2f_Dot(a_direction, b_direction);
    float angle_factor = (1.0f - dot_product) * 0.5f + 0.5f;
    
    // 质量比因子
    float mass_ratio = a->mass / b->mass;
    float mass_factor = 1.0f;
    if (mass_ratio > 1.0f) {
        mass_factor = sqrt(mass_ratio);
    }
    
    // 最终伤害计算
    float base_damage = total_energy * 0.001f;
    float final_damage = base_damage * angle_factor * mass_factor;
    
    // 添加随机性（±10%）
    float random_factor = 0.9f + (rand() % 200) / 1000.0f;
    final_damage *= random_factor;
    
    return final_damage;
}
```

## 数据配置

### CSV数据解析
物理参数从CSV文件加载：

```c
void ParsePhysicsFromCSV(Entity* entity, const char* id_name) {
    FILE* file = fopen("assets/data/ship/ship_data.csv", "r");
    char line[1024];
    
    while (fgets(line, sizeof(line), file)) {
        char* columns[16];
        int column_count = 0;
        char* token = strtok(line, ",\n");
        
        while (token && column_count < 16) {
            columns[column_count++] = token;
            token = strtok(NULL, ",\n");
        }
        
        if (column_count >= 9 && strcmp(columns[1], id_name) == 0) {
            entity->mass = atof(columns[3]);
            entity->hitpoint = atoi(columns[4]);
            entity->acceleration = atof(columns[5]);
            entity->rotational_acceleration = atof(columns[6]);
            
            // 计算转动惯量
            float bounding_radius = CalculateBoundingRadius(entity);
            entity->moment_of_inertia = entity->mass * bounding_radius * bounding_radius;
            
            break;
        }
    }
    fclose(file);
}
```

## 物理模拟循环

### 主物理更新流程
```c
void Physics_UpdateAll(float deltaTime) {
    // 1. 更新所有实体的运动
    for (每个实体) {
        Entity_Update(entity, deltaTime);
        Entity_UpdateRotation(entity, deltaTime);
    }
    
    // 2. 碰撞检测
    for (每个实体对 (i, j)) {
        if (CheckBroadPhaseCollision(entities[i], entities[j])) {
            if (CheckSATCollision(entities[i], entities[j])) {
                // 3. 碰撞响应
                ResolveCollision(entities[i], entities[j], 
                                collision_point, collision_normal);
                ResolveRotationalCollision(entities[i], entities[j],
                                          collision_point, collision_normal);
                
                // 4. 伤害计算
                float damage = CalculateCollisionDamage(entities[i], entities[j],
                                                       collision_normal);
                ApplyDamage(entities[i], entities[j], damage);
            }
        }
    }
    
    // 5. 应用边界约束
    for (每个实体) {
        ApplyWorldBoundaries(entity);
    }
}
```

### 边界处理
```c
void ApplyWorldBoundaries(Entity* entity) {
    const float WORLD_MIN_X = -5000.0f;
    const float WORLD_MAX_X = 5000.0f;
    const float WORLD_MIN_Y = -5000.0f;
    const float WORLD_MAX_Y = 5000.0f;
    
    // X轴边界
    if (entity->position.x < WORLD_MIN_X) {
        entity->position.x = WORLD_MIN_X;
        entity->velocity.x = fabs(entity->velocity.x) * 0.5f;
    } else if (entity->position.x > WORLD_MAX_X) {
        entity->position.x = WORLD_MAX_X;
        entity->velocity.x = -fabs(entity->velocity.x) * 0.5f;
    }
    
    // Y轴边界
    if (entity->position.y < WORLD_MIN_Y) {
        entity->position.y = WORLD_MIN_Y;
        entity->velocity.y = fabs(entity->velocity.y) * 0.5f;
    } else if (entity->position.y > WORLD_MAX_Y) {
        entity->position.y = WORLD_MAX_Y;
        entity->velocity.y = -fabs(entity->velocity.y) * 0.5f;
    }
}
```

## 调试功能

### 物理调试信息显示
```c
void RenderPhysicsDebugInfo(Entity* entity) {
    // 显示速度向量
    setlinecolor(RGB(0, 255, 0));
    line((int)entity->position.x, (int)entity->position.y,
         (int)(entity->position.x + entity->velocity.x),
         (int)(entity->position.y + entity->velocity.y));
    
    // 显示速度数值
    char speed_text[64];
    float speed = Vector2f_Length(entity->velocity);
    sprintf(speed_text, "Speed: %.1f", speed);
    outtextxy((int)entity->position.x, (int)entity->position.y - 20, speed_text);
    
    // 显示碰撞多边形（F1键切换）
    if (show_collision_polygons) {
        setlinecolor(RGB(255, 0, 0));
        Vector2f transformed_vertices[MAX_VERTICES];
        Polygon_GetTransformedVertices(entity->polygon, &entity->position,
                                      entity->image_center_offset, entity->angle,
                                      transformed_vertices);
        
        for (int i = 0; i < entity->polygon->vertex_count; i++) {
            int j = (i + 1) % entity->polygon->vertex_count;
            line((int)transformed_vertices[i].x, (int)transformed_vertices[i].y,
                 (int)transformed_vertices[j].x, (int)transformed_vertices[j].y);
        }
    }
}
```

这个物理系统实现了基本的牛顿力学模拟，包括线性运动、旋转运动、碰撞检测和响应，为游戏提供了真实的物理交互。