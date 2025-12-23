#include "Entity.h"

// 重构的CSV读取创建实体
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
            // 默认情况下使用 ship 路径，或者可以根据需求修改
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
    float width = image_center_offset.x*2;
    float height = image_center_offset.y*2;
    entity->moment_of_inertia = entity->mass * (width*width + height*height) / 12.0f;
    // 初始化变换后的顶点
    if (polygon && polygon->vertex_count > 0) {
        entity->transformed_vertices = (Vector2f*)malloc(
                polygon->vertex_count * sizeof(Vector2f)
        );
        if (!entity->transformed_vertices) {
            fprintf(stderr, "Error: Failed to allocate transformed vertices\n");
            free(entity);
        }
        Polygon_GetTransformedVertices(
                polygon,
                &entity->position,
                entity->image_center_offset,
                entity->angle,
                entity->transformed_vertices
        );
    } else {
        entity->transformed_vertices = NULL;
    }
}

void Entity_AddId(Entity* entity, int id){
    entity->id = id;
}

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

void Entity_RotateLeft(Entity* entity, float deltaTime) {
    // 左转时减少角度（顺时针旋转）
    entity->rotational_velocity = -entity->rotational_acceleration;
}

void Entity_RotateRight(Entity* entity, float deltaTime) {
    // 右转时增加角度（逆时针旋转）
    entity->rotational_velocity = entity->rotational_acceleration;
}


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

void Entity_Update_Unimpeded(Entity* entity, float deltaTime) {

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

        // ======================
        // 1. 统一使用物理坐标系（Y轴向上）
        // ======================
        // 将分离向量转换到物理坐标系
        Vector2f physics_separation = separation_vector;
        physics_separation.y *= -1; // 反转Y轴

        // ======================
        // 2. 冲量计算
        // ======================
        Vector2f separation_dir = physics_separation;
        Vector2f_normalize(&separation_dir);

        float total_mass = entity->mass + otherEntity->mass;
        float epsilon = 0.01;
        float e = 0.8f; // 更合理的恢复系数

        // 速度投影（已在物理坐标系）
        float v1_sep = Vector2f_dot(&entity->velocity, &separation_dir);
        float v2_sep = Vector2f_dot(&otherEntity->velocity, &separation_dir);
        float v_rel = v1_sep - v2_sep;

        float j = -(1 + e) * v_rel / (1/entity->mass + 1/otherEntity->mass + epsilon);

        Vector2f impulse;
        Vector2f_scale(&separation_dir, j, &impulse);

        // ======================
        // 3. 应用线速度变化
        // ======================
        Vector2f delta_v1, delta_v2;
        Vector2f_scale(&impulse, 1.0f/entity->mass, &delta_v1);
        Vector2f_scale(&impulse, -1.0f/otherEntity->mass, &delta_v2);

        Vector2f_add(&entity->velocity, &delta_v1, &entity->velocity);
        Vector2f_add(&otherEntity->velocity, &delta_v2, &otherEntity->velocity);

        // ======================
        // 4. 角速度计算（关键修正部分）
        // ======================
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

        // 物理坐标系下的冲量（无需调整Y轴）
        Vector2f physics_impulse = impulse;

        // 计算力矩（标准叉积公式 τ = r × F）
        float cross_entity = r_entity.x * physics_impulse.y - r_entity.y * physics_impulse.x;
        float cross_other = r_other.x * physics_impulse.y - r_other.y * physics_impulse.x;

        // 更新角速度（注意转动惯量单位）
        entity->rotational_velocity -= 50* cross_entity / (entity->moment_of_inertia + epsilon);
        otherEntity->rotational_velocity += 50* cross_other / (otherEntity->moment_of_inertia + epsilon);

        // ======================
        // 5. 位置修正（使用原始坐标系）
        // ======================
        float position_correction = 0.2f;
        Vector2f correction1, correction2;
        Vector2f_scale(&separation_vector, -position_correction, &correction1);
        Vector2f_scale(&separation_vector, position_correction, &correction2);

        Vector2f_add(&entity->position, &correction1, &entity->position);
        Vector2f_add(&otherEntity->position, &correction2, &otherEntity->position);

        return true;
    }
    return false;
}

/*
// 碰撞处理
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
        // ======================
        // 1. 坐标系适配（EasyX的Y轴向下）
        // ======================
        Vector2f separation_dir = separation_vector;
        Vector2f_normalize(&separation_dir);
        separation_dir.y *= -1; // 反转Y轴方向以适配EasyX坐标系

        // ======================
        // 2. 冲量计算与应用
        // ======================
        float total_mass = entity->mass + otherEntity->mass;
        float epsilon = 1e-6; // 防止除零的小常数

        // 沿分离方向的速度分量
        float v1_sep = Vector2f_dot(&entity->velocity, &separation_dir);
        float v2_sep = Vector2f_dot(&otherEntity->velocity, &separation_dir);

        // 恢复系数（e），可以适当调低以减少反弹强度
        float e = 1.0f; // 调整恢复系数

        // 相对速度
        float v_rel = v1_sep - v2_sep;

        // 计算冲量
        float j = -(1 + e) * v_rel / (1/entity->mass + 1/otherEntity->mass + 0.01);

        // 应用冲量
        Vector2f impulse;
        Vector2f_scale(&separation_dir, j, &impulse);

        // 保存原始冲量用于角速度计算（注意Y轴方向）
        Vector2f original_impulse = impulse;
        original_impulse.y *= -1; // 恢复原始坐标系方向
        Vector2f entity_center, other_center;
        Vector2f_add(&entity->position, &entity->image_center_offset, &entity_center);
        Vector2f_add(&otherEntity->position, &otherEntity->image_center_offset, &other_center);

        Vector2f delta_v1, delta_v2;
        Vector2f_scale(&impulse, 1.0f/entity->mass, &delta_v1); // 瞬时冲量
        Vector2f_scale(&impulse, -1.0f/otherEntity->mass, &delta_v2); // 瞬时冲量

        Vector2f_add(&entity->velocity, &delta_v1, &entity->velocity);
        Vector2f_add(&otherEntity->velocity, &delta_v2, &otherEntity->velocity);

        // ======================
        // 新增：角速度变化计算
        // ======================

        Vector2f r_entity, r_other;
        Vector2f_subtract(&collision_point, &entity_center, &r_entity);
        Vector2f_subtract(&collision_point, &other_center, &r_other);

        // 调整冲量方向到物理坐标系（Y轴向上）
        Vector2f physics_impulse = impulse;
        physics_impulse.y *= 1;


// 计算力矩（叉乘公式调整为：τ = r.x * F.y - r.y * F.x）
        float cross_entity = r_entity.x * physics_impulse.y - r_entity.y * physics_impulse.x;
        float cross_other = r_other.x * (physics_impulse.y) - r_other.y * (physics_impulse.x);

// 更新角速度
        entity->rotational_velocity += cross_entity*100 / (entity->moment_of_inertia + epsilon);
        otherEntity->rotational_velocity -= cross_other*100 / (otherEntity->moment_of_inertia + epsilon);

        // ======================
        // 3. 位置修正防止穿透
        // ======================

        float position_correction_factor = 0.5f; // 调整这个值以控制位置修正的强度
        Vector2f position_correction1, position_correction2;
        Vector2f_scale(&separation_vector, -position_correction_factor, &position_correction1);
        Vector2f_scale(&separation_vector, position_correction_factor, &position_correction2);

        Vector2f_add(&entity->position, &position_correction1, &entity->position);
        Vector2f_add(&otherEntity->position, &position_correction2, &otherEntity->position);
        return true;
    }
    return false;
}

 */

void Entity_ShowPolygon(Entity* entity) {
    if (entity->polygon == NULL || entity->polygon->vertex_count < 3 ||
        entity->transformed_vertices == NULL) {
        return;
    }
    BeginBatchDraw();
    // 绘制多边形
    POINT points[100];
    for (int i = 0; i < entity->polygon->vertex_count; i++) {
        points[i].x = (int)(entity->transformed_vertices[i].x);
        points[i].y = (int)(entity->transformed_vertices[i].y);
    }
    points[entity->polygon->vertex_count] = points[0];

    setlinecolor(WHITE);
    setlinestyle(PS_SOLID,2,0);
    polyline(points, entity->polygon->vertex_count + 1);

    // 绘制中心点
    setfillcolor(YELLOW);
    fillellipse(
            (int)entity->position.x - 2,
            (int)entity->position.y - 2,
            (int)entity->position.x + 2,
            (int)entity->position.y + 2
    );

    // ======================
    // 新增调试信息
    // ======================
    // 显示速度向量
    setlinecolor(GREEN);
    setlinestyle(PS_SOLID,5,0);
    line(
            (int)entity->position.x,
            (int)entity->position.y,
            (int)(entity->position.x + entity->velocity.x * 5),
            (int)(entity->position.y + entity->velocity.y * 5)
    );


    // 显示速度（文本）
    char info[128];
    sprintf(info, "velocity:%.1f, %.1f", entity->velocity.x, entity->velocity.y);
    settextcolor(LIGHTGRAY);
    outtextxy(
            (int)entity->position.x + 15,
            (int)entity->position.y - 10,
            info
    );

    // 显示角速度
    sprintf(info, "rotational_velocity:%.1f", entity->rotational_velocity);
    outtextxy(
            (int)entity->position.x + 15,
            (int)entity->position.y - 25,
            info
    );

    // 显示转动惯量
    sprintf(info, "moment_of_inertia:%.1f", entity->moment_of_inertia);
    outtextxy(
            (int)entity->position.x + 15,
            (int)entity->position.y - 40,
            info
    );

    // 显示质量信息
    sprintf(info, "mass:%d", entity->mass);
    outtextxy(
            (int)entity->position.x + 15,
            (int)entity->position.y + 5,
            info
    );
    FlushBatchDraw();
}

void Entity_Light(Entity* entity){
    ImageProcessing_AdjustImageTransparency(&entity->light_controller.light_image_after_adjust, &entity->light_controller.light_image, entity->light_controller.intensity);
    ImageProcessing_BlendImages(&entity->image_with_light, &entity->light_controller.light_image_after_adjust, &entity->image);
}

void Entity_Destroy(Entity* entity){
    // 所有数据清空
    entity->id = 0;
//    entity->id_name = NULL;
//    entity->image = NULL;
    entity->image_center_offset.x = 0;
    entity->image_center_offset.y = 0;
//    entity->image_with_light = NULL;
    entity->light_controller.intensity = 0;
//    entity->light_controller.light_image = NULL;
//    entity->light_controller.light_image_after_adjust = NULL;
    entity->mass = 0;
    entity->moment_of_inertia = 0;
//    entity->polygon = NULL;
    entity->position.x = 0;
    entity->position.y = 0;
    entity->rotational_velocity = 0;
    entity->velocity.x = 0;
    entity->velocity.y = 0;
    entity->angle = 0;
    entity->acceleration = 0;
    entity->rotational_acceleration = 0;
//    entity->transformed_vertices = NULL;
    entity->hitpoint = 0;
}