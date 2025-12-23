// TacticalMap.c
#include "TacticalMap.h"

#include <stdio.h>
#include <math.h>

// 配置结构
static struct {
    // 地图配置
    int map_size = 2048;  // 2048x2048的世界地图
    int display_size = 200; // 战术地图显示大小(600x600)
    int pos_x = 1280 - 276; // 右侧位置
    int pos_y = 70;        // 顶部位置

    // 缩放比例(显示大小/实际大小)
    float scale = 200.0f / 2048.0f;

    // 颜色配置
    COLORREF bg_color = RGB(20, 40, 20);      // 深绿色背景
    COLORREF grid_color = RGB(50, 100, 50);   // 浅绿色网格
    COLORREF ally_color = RGB(100, 255, 100); // 亮绿色友军
    COLORREF enemy_color = RGB(255, 100, 100);// 红色敌军
    COLORREF text_color = RGB(200, 255, 200); // 浅绿色文字

    // 单位显示设置
    int unit_size = 10;             // 单位点大小
    int direction_line_length = 30; // 方向线长度
} s_config;

static bool s_visible = false;

void TacticalMap_Init() {
    s_visible = false;
}

bool TacticalMap_Update() {
    static bool last_state = false;
    bool current_state = GetAsyncKeyState(VK_TAB) & 0x8000;

    if (current_state && !last_state) {
        s_visible = !s_visible;
    }

    last_state = current_state;
    return s_visible;
}

// 将任意角度规范化为-π到π之间的弧度
static float normalize_angle(float angle_degrees) {
    // 转换为弧度(你的0度是12点方向，相当于-90度)
    float radians = (angle_degrees - 90.0f) * (3.14159265f / 180.0f);

    // 规范化到[-π, π]范围
    while (radians > 3.14159265f) radians -= 2 * 3.14159265f;
    while (radians <= -3.14159265f) radians += 2 * 3.14159265f;

    return radians;
}

void TacticalMap_Render(ResourceManager* rm, Camera* camera, ShipAPI* player, const CombatDataPool* pool) {
    if (!s_visible) return;

    // 保存当前绘图设置
    int old_fill_color = getfillcolor();
    int old_line_color = getcolor();
    int old_bk_mode = getbkmode();
    int old_text_color = gettextcolor();


    // 设置透明文本背景
    setbkmode(TRANSPARENT);

    setfillcolor(s_config.bg_color);
    bar(s_config.pos_x, s_config.pos_y,
        s_config.pos_x + s_config.display_size,
        s_config.pos_y + s_config.display_size);

    // 绘制网格(每256单位一格)
    setcolor(s_config.grid_color);
    for (int x = 0; x <= s_config.map_size; x += 256) {
        int display_x = s_config.pos_x + (int)(x * s_config.scale);
        line(display_x, s_config.pos_y,
             display_x, s_config.pos_y + s_config.display_size);
    }
    for (int y = 0; y <= s_config.map_size; y += 256) {
        int display_y = s_config.pos_y + (int)(y * s_config.scale);
        line(s_config.pos_x, display_y,
             s_config.pos_x + s_config.display_size, display_y);
    }

    // 绘制边框
    rectangle(s_config.pos_x, s_config.pos_y,
              s_config.pos_x + s_config.display_size,
              s_config.pos_y + s_config.display_size);

    // 绘制标题
    settextcolor(s_config.text_color);
    setfont(20, 0, "Arial");
    outtextxy(s_config.pos_x + 10, s_config.pos_y + 10, "Tactical Map");

    setfont(30, 0, "Arial");
    outtextxy(60, 300, player->entity_data.id_name);
    setfont(20, 0, "Arial");
    outtextxy(60, 350, "ship state:");
    setfont(30, 0, "Arial");
    outtextxy(300, 550, "weapon name:");
    outtextxy(camera->screen_position.x+59, camera->screen_position.y-80, "AIM POSITION");
    setfont(20, 0, "Arial");

    for(int i = 0; i < player->weapon_count; i++){
        outtextxy(350, 600 + i*20, player->weapon[i].name);
        if(player->weapon[i].fire_manager.canFire){
            outtextxy(450, 600 + i*20, "can fire");
        }else{
            outtextxy(450, 600 + i*20, "charging");
        }
    }

    // 绘制单位
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (!pool->data[i].active) continue;

        // 计算地图位置(允许超出边界)
        int x = s_config.pos_x + (int)((pool->data[i].position.x + 1024) * s_config.scale);
        int y = s_config.pos_y + (int)((pool->data[i].position.y + 1024) * s_config.scale);

        // 选择颜色
        COLORREF unit_color;
        if (pool->data[i].team == 1) {
            unit_color = s_config.ally_color;  // 友军
        } else {
            unit_color = s_config.enemy_color; // 敌军
        }

        // 绘制单位
        setfillcolor(unit_color);
        setcolor(unit_color);
        fillcircle(x, y, s_config.unit_size);

        // 计算方向(处理角度规范化和方向)
        float angle_rad = normalize_angle(pool->data[i].angle);
        int end_x = x + (int)(s_config.direction_line_length * cosf(angle_rad));
        int end_y = y + (int)(s_config.direction_line_length * sinf(angle_rad));

        // 绘制方向线
        line(x, y, end_x, end_y);

        // 显示血量
        char hp_text[16];
        sprintf(hp_text, "%.0f", pool->data[i].hp);
        settextcolor(unit_color);
        outtextxy(x + 8, y - 8, hp_text);
    }

    IMAGE ship_graph;
    ResourceManager_GetIMAGE(rm, &ship_graph, "UI", player->entity_data.id_name);
    RenderManager_Draw(200, 500, &ship_graph, player->entity_data.angle);

    IMAGE UI_graph;
    ResourceManager_GetIMAGE(rm, &UI_graph, "UI", "UI");
    RenderManager_Draw(640, 360, &UI_graph, 0);

    IMAGE ship_UI_graph;
    ResourceManager_GetIMAGE(rm, &ship_UI_graph, "UI", "ship_UI");
    RenderManager_Draw(camera->screen_position.x, camera->screen_position.y, &ship_UI_graph, 0);


    // 绘制图例
    settextcolor(s_config.text_color);
    setfont(14, 0, "Arial");
    outtextxy(s_config.pos_x + 10, s_config.pos_y + s_config.display_size - 60, "Allies");
    setfillcolor(s_config.ally_color);
    fillcircle(s_config.pos_x + 60, s_config.pos_y + s_config.display_size - 55, 4);

    outtextxy(s_config.pos_x + 10, s_config.pos_y + s_config.display_size - 40, "Enemies");
    setfillcolor(s_config.enemy_color);
    fillcircle(s_config.pos_x + 60, s_config.pos_y + s_config.display_size - 35, 4);

    // 恢复绘图设置
    setfillcolor(old_fill_color);
    setcolor(old_line_color);
    setbkmode(old_bk_mode);
    settextcolor(old_text_color);
}