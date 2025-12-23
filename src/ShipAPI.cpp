#include "ShipAPI.h"
#include "ExplosionAPI.h"


ShipAPI* ShipAPI_Init(RenderManager* rdm,
                  ResourceManager* rm,
                  const char* name,
                  Vector2f position,
                  float angle,
                  Vector2f velocity,
                  bool isPlayer){

    ShipAPI* ship = new(ShipAPI);

    Entity_Create_FromCSV(rm,
                          &ship->entity_data,
                          LoadPolygonFromJSON(name, SHIP),
                          position,
                          angle,
                          velocity,
                          name,
                          SHIP);

    ship->isPlayer = isPlayer;

    // 构建文件路径
    char filename[256];
    snprintf(filename, sizeof(filename), "assets/data/ship/%s.json", name);

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

    cJSON* engines = cJSON_GetObjectItem(root, "engines");
    if (!engines || !cJSON_IsArray(engines)) {
        cJSON_Delete(root);
    }
    int engine_count = cJSON_GetArraySize(engines);
    ship->engine_count = engine_count;
    for (int i = 0; i < engine_count; i++) {
        cJSON* item = cJSON_GetArrayItem(engines, i);
        Engine_Init(&ship->engine[i],
                    rm,
                    cJSON_GetObjectItem(item, "name")->valuestring);
        // 获取 attachment_point 对象
        cJSON* attachment_point_obj = cJSON_GetObjectItem(item, "attachment_point");
        ship->engine[i].offset_position.x = (float)cJSON_GetObjectItem(attachment_point_obj, "x")->valuedouble;
        ship->engine[i].offset_position.y = (float)cJSON_GetObjectItem(attachment_point_obj, "y")->valuedouble;
        ship->engine[i].offset_angle = cJSON_GetObjectItem(item, "attachment_angle_offset")->valuedouble;
    }

    Entity_AddId(&ship->entity_data, RenderManager_AddEntity(rdm, &ship->entity_data.image, position.x, position.y, angle));
    for (int i = 0; i < engine_count; i++) {
        Engine_AddId(&ship->engine[i],
                     RenderManager_AddEffect(rdm,
                                             &ship->engine[i].image,
                                             ship->engine[i].position.x,
                                             ship->engine[i].position.y,
                                             ship->engine[i].angle));
    }
    // 武器初始化 ================================================
    cJSON* weapons = cJSON_GetObjectItem(root, "weapons");
    if (weapons && cJSON_IsArray(weapons)) {
        int weapon_count = cJSON_GetArraySize(weapons);
        weapon_count = weapon_count > 10 ? 10 : weapon_count; // 限制最大武器数量

        ship->weapon_count = weapon_count;

        for (int i = 0; i < weapon_count; i++) {
            cJSON* weapon_item = cJSON_GetArrayItem(weapons, i);

            // 解析武器基本信息
            const char* weapon_name = cJSON_GetObjectItem(weapon_item, "name")->valuestring;
            float weapon_angle = (float)cJSON_GetObjectItem(weapon_item, "angle")->valuedouble;

            // 创建武器实例
            WeaponAPI* wp = WeaponAPI_create_fromCSV(rm, weapon_name);
            wp->original_ship_id = ship->entity_data.id;
            // 设置挂载点信息
            cJSON* attach_point = cJSON_GetObjectItem(weapon_item, "attachment_point");
            wp->original_position.x = (float)cJSON_GetObjectItem(attach_point, "x")->valuedouble;
            wp->original_position.y = (float)cJSON_GetObjectItem(attach_point, "y")->valuedouble;

            // 设置偏移量（来自JSON）
            cJSON* offset = cJSON_GetObjectItem(weapon_item, "offset");
            wp->offset.x = (float)cJSON_GetObjectItem(offset, "x")->valuedouble;
            wp->offset.y = (float)cJSON_GetObjectItem(offset, "y")->valuedouble;

            // 计算初始世界坐标（考虑飞船旋转）
            float rad = angle * (M_PI / 180.0f);
            Vector2f rotated_pos = {
                    wp->original_position.x * cosf(rad) - wp->original_position.y * sinf(rad),
                    wp->original_position.x * sinf(rad) + wp->original_position.y * cosf(rad)
            };

            // 更新武器最终位置
            Vector2f_add(&position, &rotated_pos, &wp->position);
            wp->angle = angle + weapon_angle;

            // 添加到渲染管理器
            wp->id = RenderManager_AddWeapon(rdm, &wp->image, wp->position.x, wp->position.y, wp->angle);

            // 保存到飞船武器数组
            ship->weapon[i] = *wp;
            free(wp); // 释放临时武器对象
        }
    } else {
        fprintf(stderr, "Warning: No valid weapons array found\n");
        ship->weapon_count = 0;
    }

    cJSON_Delete(root); // 清理JSON解析器
    return ship;
};

void ShipAPI_AddTeam(ShipAPI* ship, int team){
    ship->team = team;
}

void ShipAPI_Update(ShipAPI* ship, Camera* camera, float deltaTime, Vector2f position) {
    for (int i = 0; i < ship->engine_count; i++) {
        Engine_Attachment(&ship->engine[i],
                          &ship->entity_data,
                          ship->engine[i].offset_position,
                          ship->engine[i].offset_angle);
        Engine_Update(&ship->engine[i]);
    }

    Entity_Update(&ship->entity_data, deltaTime);

    for (int i = 0; i < ship->weapon_count; i++) {
        WeaponAPI *wp = &ship->weapon[i];
        Entity *entity = &ship->entity_data;


        // 1. 计算基础挂载点位置（考虑贴图中心偏移）
        Vector2f base_position;
        Vector2f_subtract(&wp->original_position, &entity->image_center_offset, &base_position);

        // 2. 应用飞船旋转
        Vector2f rotated_position;
        Vector2f_rotate(&base_position, entity->angle, &rotated_position);

        // 3. 计算最终世界坐标（飞船位置 + 旋转后的位置 + 武器偏移）
        Vector2f final_position;
        Vector2f_add(&entity->position, &rotated_position, &final_position);
        Vector2f_add(&final_position, &wp->offset, &final_position);


        // 4. 更新武器属性
        wp->position = final_position;
        if(ship->isPlayer){
            WeaponAPI_FollowMouse(wp, camera);
        }else{
            WeaponAPI_Aim(wp, position);
        }

        if(wp->isFixed){
            wp->final_angle = entity->angle + wp->angle;
            // 计算最终开火位置
            Vector2f rotated_fire_position;
            Vector2f_rotate(&wp->original_fire_position, wp->final_angle, &rotated_fire_position);
            Vector2f_add(&final_position, &rotated_fire_position, &wp->fire_position);
        }else{
            // 计算最终开火位置
            Vector2f rotated_fire_position;
            Vector2f_rotate(&wp->original_fire_position, entity->angle + wp->final_angle, &rotated_fire_position);
            Vector2f_add(&final_position, &rotated_fire_position, &wp->fire_position);
        }

        // 更新冷却管理器
        WeaponAPI_UpdateFireManager(wp, deltaTime);
    }
}

void ShipAPI_Render(RenderManager* rdm, ShipAPI* ship){
    for(int i = 0; i < ship->engine_count; i++){
        RenderManager_Update(rdm,
                             ship->engine[i].id,
                             ship->engine[i].position.x,
                             ship->engine[i].position.y,
                             ship->engine[i].angle,
                             &ship->engine[i].image_final_render);
    }
    for(int i = 0; i < ship->weapon_count; i++){
        RenderManager_Update(rdm,
                             ship->weapon[i].id,
                             ship->weapon[i].position.x,
                             ship->weapon[i].position.y,
                             ship->weapon[i].final_angle,
                             &ship->weapon[i].image);
    }
    RenderManager_Update(rdm,
                         ship->entity_data.id,
                         ship->entity_data.position.x,
                         ship->entity_data.position.y,
                         ship->entity_data.angle,
                         &ship->entity_data.image_with_light);
}

void ShipAPI_Forward(ShipAPI* ship, float deltaTime){
    Entity_Forward(&ship->entity_data, deltaTime);
    for(int i = 0; i < ship->engine_count; i++){
        Engine_Advance(&ship->engine[i]);
    }

    if(ship->entity_data.light_controller.intensity < 0.98){
        ship->entity_data.light_controller.intensity += 0.01;
    }
}

void ShipAPI_EngineDown(ShipAPI* ship){
    for(int i = 0; i < ship->engine_count; i++){
        Engine_Stop(&ship->engine[i]);
    }
    if(ship->entity_data.light_controller.intensity > 0.1){
        ship->entity_data.light_controller.intensity -= 0.01;
    }
}

void ShipAPI_Backward(ShipAPI* ship, float deltaTime){
    Entity_Backward(&ship->entity_data, deltaTime);
}

void ShipAPI_TurnLeft(ShipAPI* ship, float deltaTime){
    Entity_RotateLeft(&ship->entity_data, deltaTime);
}

void ShipAPI_TurnRight(ShipAPI* ship, float deltaTime){
    Entity_RotateRight(&ship->entity_data, deltaTime);
}

void ShipAPI_Destroy(ResourceManager* rsm, RenderManager* rdm, ShipAPI* ship){
    ExplosionAPI_CreateExplosion(rsm, rdm, EXPLOSION_SHIP, ship->entity_data.position, 0.5);
    RenderManager_DeleteNode(rdm, ship->entity_data.id);
    RenderManager_DeleteIdNode(rdm, ship->entity_data.id);
    Entity_Destroy(&ship->entity_data);
    for(int i = 0; i < ship->engine_count; i++){
        RenderManager_DeleteNode(rdm, ship->engine[i].id);
        RenderManager_DeleteIdNode(rdm, ship->engine[i].id);
    }
    for(int i = 0; i < ship->weapon_count; i++){
        RenderManager_DeleteNode(rdm, ship->weapon[i].id);
        RenderManager_DeleteIdNode(rdm, ship->weapon[i].id);
    }
    delete(ship);
}
