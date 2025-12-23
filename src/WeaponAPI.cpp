#include "WeaponAPI.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

// 从JSON文件中读取WeaponAPI对象的数据
static void WeaponAPI_LoadFromJSON(WeaponAPI* weapon, const char* weaponName, ResourceManager* rsm) {
    char jsonPath[256];
    snprintf(jsonPath, sizeof(jsonPath), "assets/data/weapon/%s.json", weaponName);

    FILE* file = fopen(jsonPath, "r");
    if (!file) {
        fprintf(stderr, "Error: Failed to open JSON file %s\n", jsonPath);
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    fread(buffer, 1, fileSize, file);
    fclose(file);
    buffer[fileSize] = '\0';

    cJSON* root = cJSON_Parse(buffer);
    free(buffer);

    if (!root) {
        fprintf(stderr, "Error: Failed to parse JSON file %s\n", jsonPath);
        return;
    }

    cJSON* isLazerItem = cJSON_GetObjectItemCaseSensitive(root, "isLazer");
    cJSON* isBulletItem = cJSON_GetObjectItemCaseSensitive(root, "isBullet");
    cJSON* isMissileItem = cJSON_GetObjectItemCaseSensitive(root, "isMissile");
    cJSON* bulletNameItem = cJSON_GetObjectItemCaseSensitive(root, "BulletName");
    cJSON* OriginalFirePositionItem = cJSON_GetObjectItemCaseSensitive(root, "OriginalFirePosition");

    if (isLazerItem && isLazerItem->type == cJSON_True) {
        weapon->isLazer = true;
    } else {
        weapon->isLazer = false;
    }

    if (isBulletItem && isBulletItem->type == cJSON_True) {
        weapon->isBullet = true;
    } else {
        weapon->isBullet = false;
    }

    if (isMissileItem && isMissileItem->type == cJSON_True) {
        weapon->isMissile = true;
    } else {
        weapon->isMissile = false;
    }

    if (bulletNameItem && bulletNameItem->type == cJSON_String) {
        weapon->bullet_name = strdup(bulletNameItem->valuestring);
    } else {
        weapon->bullet_name = NULL;
    }

    cJSON* xItem = cJSON_GetArrayItem(OriginalFirePositionItem, 0);
    cJSON* yItem = cJSON_GetArrayItem(OriginalFirePositionItem, 1);

    if (xItem && xItem->type == cJSON_Number && yItem && yItem->type == cJSON_Number) {
        weapon->original_fire_position.x = xItem->valuedouble;
        weapon->original_fire_position.y = yItem->valuedouble;
    }

    cJSON_Delete(root);
}

// 从CSV文件中创建WeaponAPI对象
WeaponAPI* WeaponAPI_create_fromCSV(ResourceManager* rsm, const char* weaponName) {
    FILE* file = fopen("assets/data/weapon/weapon_data.csv", "r");
    if (!file) {
        fprintf(stderr, "Error: Failed to open weapon_data.csv\n");
        return NULL;
    }

    char line[1024];
    int found = 0;
    WeaponAPI* weapon = new WeaponAPI;

    // 跳过第一行（标题行）
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file)) {
        char* columns[16];
        int column_count = 0;
        char* token = strtok(line, ",\n");

        // 分割每行的字段
        while (token && column_count < 16) {
            columns[column_count++] = token;
            token = strtok(NULL, ",\n");
        }

        // 检查列数并匹配id_name
        if (column_count >= 7 && strcmp(columns[1], weaponName) == 0) { // id_name对应name

            weapon->name = strdup(weaponName);
            weapon->id = -1;
            weapon->rotate_acceleration = atof(columns[2]);
            weapon->cooldown = atof(columns[3]);
            weapon->range = atof(columns[4]);
            weapon->offset.x = atof(columns[5]);
            weapon->offset.y = atof(columns[6]);
            if(atof(columns[7]) == 1){
                weapon->isFixed = true;
            }

            // 其他字段通过JSON读取
            WeaponAPI_LoadFromJSON(weapon, weaponName, rsm);

            ResourceManager_GetIMAGE(rsm, &weapon->image, "weapon", weaponName);
            ResourceManager_GetIMAGE(rsm, &weapon->glow, "weapon_glow", weaponName);

            weapon->fire_manager.cooldown_time = weapon->cooldown;
            found = 1;
            break;
        }
    }

    fclose(file);

    if (!found) {
        fprintf(stderr, "Error: Weapon '%s' not found in CSV\n", weaponName);
        return NULL;
    }

    return weapon;
}
void WeaponAPI_FollowMouse(WeaponAPI* weapon, Camera* camera) {
    if (!weapon || !camera) return;

    if (!weapon->isFixed) {
        // 1. 计算屏幕中心位置（玩家位置）
        int screenCenterX = 1280 / 2;
        int screenCenterY = 720 / 2;

        // 2. 计算武器在屏幕上的位置（基于玩家中心的视角）
        Vector2f weaponScreenPos = {
                screenCenterX + (weapon->position.x - camera->player_position.x) * ENTITY_EFFECT_OFFSET,
                screenCenterY + (weapon->position.y - camera->player_position.y) * ENTITY_EFFECT_OFFSET
        };

        // 3. 鼠标位置保持不变（直接使用camera->screen_position）
        Vector2f mouseScreenPos = camera->screen_position;

        // 4. 计算方向向量（Y轴向下需取反）
        float dx = mouseScreenPos.x - weaponScreenPos.x;
        float dy = mouseScreenPos.y - weaponScreenPos.y;

        // 5. 计算角度
        float targetAngle = atan2f(dy, dx) * 180.0f / PI;
        weapon->final_angle = 90 + fmodf(targetAngle + 360.0f, 360.0f);
    }
}
void WeaponAPI_Aim(WeaponAPI* weapon, Vector2f position) {
    if (!weapon) return;

    if (!weapon->isFixed) {
        // 1. 计算方向向量（Y轴向下需取反）
        float dx = position.x - weapon->position.x;
        float dy = position.y - weapon->position.y;

        // 2. 计算角度
        float targetAngle = atan2f(dy, dx) * 180.0f / PI;
        weapon->final_angle = 90 + fmodf(targetAngle + 360.0f, 360.0f);
    }
}

void WeaponAPI_UpdateFireManager(WeaponAPI* weapon, float deltaTime){

    if (weapon->fire_manager.cooldown_time <= 0){
        weapon->fire_manager.cooldown_time = 0;
        weapon->fire_manager.canFire = true;
    }else {
        weapon->fire_manager.cooldown_time -= deltaTime;
    }
}

bool WeaponAPI_CanFire(WeaponAPI* weapon){
    return weapon->fire_manager.canFire;
}
