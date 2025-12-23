#include "BulletAPI.h"


BulletAPI* BulletAPI_Init(RenderManager* rdm, ResourceManager* rsm, const char* name, WeaponAPI* weapon) {
    BulletAPI* bullet = new(BulletAPI);
    bullet->entity_data.id_name = name;
    bullet->out = false;

    Timer_Init(&bullet->timer);
    Timer_Start(&bullet->timer);

    // 构建文件路径
    char filename[256];
    snprintf(filename, sizeof(filename), "assets/data/bullet/%s.json", name);

    // 读取文件
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Hitbox file %s not found\n", filename);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* data = (char*)malloc(length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = 0;

    // 解析JSON
    cJSON* root = cJSON_Parse(data);
    free(data);
    if (!root) {
        fprintf(stderr, "Error: Invalid JSON in %s\n", filename);
    }

    bullet->damage = cJSON_GetObjectItem(root, "damage")->valueint;

    Vector2f velocity;
    Vector2f firePosition;
    float velocity_multiple = cJSON_GetObjectItem(root, "velocity")->valuedouble;
    float angle_in_radians = (weapon->final_angle-90) * (M_PI / 180.0);
    velocity.x = cos(angle_in_radians) * velocity_multiple;
    velocity.y = sin(angle_in_radians) * velocity_multiple;

    cJSON* explosionTypeItem = cJSON_GetObjectItemCaseSensitive(root, "ExplosionType");

    // 设置默认爆炸类型
    bullet->explosionType = EXPLOSION_SHELL; // 默认值

    if (explosionTypeItem && explosionTypeItem->type == cJSON_String) {
        const char* typeStr = explosionTypeItem->valuestring;

        if (strcmp(typeStr, "EXPLOSION_SHIP") == 0) {
            bullet->explosionType = EXPLOSION_SHIP;
        } else if (strcmp(typeStr, "EXPLOSION_SHELL") == 0) {
            bullet->explosionType = EXPLOSION_SHELL;
        } else if (strcmp(typeStr, "EXPLOSION_FIRE") == 0) {
            bullet->explosionType = EXPLOSION_FIRE;
        } else if (strcmp(typeStr, "EXPLOSION_HEAVY_ONHIT") == 0) {
            bullet->explosionType = EXPLOSION_HEAVY_ONHIT;
        } else if (strcmp(typeStr, "EXPLOSION_LASER_FIRE") == 0) {
            bullet->explosionType = EXPLOSION_LASER_FIRE;
        } else if (strcmp(typeStr, "EXPLOSION_LASER_HIT") == 0) {
            bullet->explosionType = EXPLOSION_LASER_HIT;
        }
    }

    Entity_Create_FromCSV(rsm,
                          &bullet->entity_data,
                          LoadPolygonFromJSON(name, BULLET),
                          weapon->fire_position,
                          weapon->final_angle,
                          velocity,
                          name,
                          BULLET);
    bullet->original_ship_id = weapon->original_ship_id;
    Entity_AddId(&bullet->entity_data, RenderManager_AddEntity(rdm,
                                                               &bullet->entity_data.image,
                                                               bullet->entity_data.position.x,
                                                               bullet->entity_data.position.y,
                                                               bullet->entity_data.angle));
}

void BulletAPI_Update(BulletAPI* bullet, float deltaTime) {
    Entity_Update_Unimpeded(&bullet->entity_data, deltaTime);
    if(Timer_GetElapsedSeconds(&bullet->timer) >= 3){
        bullet->out = true;
    }
    if(Timer_GetElapsedMilliseconds(&bullet->timer) > 200){
        bullet->noCollision = false;
    }
}

void BulletAPI_Render(RenderManager* rdm, BulletAPI* bullet){
    RenderManager_Update(rdm,
                         bullet->entity_data.id,
                         bullet->entity_data.position.x,
                         bullet->entity_data.position.y,
                         bullet->entity_data.angle,
                         &bullet->entity_data.image);
}

//void BulletAPI_Destroy(RenderManager* rm, BulletAPI* bullet){
//    RenderManager_DeleteNode(rm, bullet->entity_data.id);
//    Entity_Destroy(&bullet->entity_data);
//    Timer_Destroy(&bullet->timer);
//    bullet->out = false;
//}

void BulletAPI_Destroy(RenderManager* rm, BulletAPI* bullet) {
    if(!bullet) return;

    // 释放渲染资源
    if(rm && bullet->entity_data.id >= 0) {
        RenderManager_DeleteNode(rm, bullet->entity_data.id);
        RenderManager_DeleteIdNode(rm, bullet->entity_data.id);
    }

    Timer_Destroy(&bullet->timer);
    // 释放Entity相关资源
    Entity_Destroy(&bullet->entity_data);

//    // 释放子弹内存
//    free(bullet);
}
