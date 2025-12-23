#ifndef STARSECTOR_NANO_CAMERA_H
#define STARSECTOR_NANO_CAMERA_H

#include <graphics.h>
#include "MathTool.h"
#include "Vector2f.h"

// 摄像机
typedef struct {
    Vector2f position; // 摄像机世界位置
    Vector2f screen_position; // 摄像机在屏幕上的位置
    Vector2f player_position = {0, 0};
} Camera;

// 获取鼠标坐标并返回Vector2f类型
Vector2f Camera_GetMousePosition();

void Camera_UpdatePlayerPosition(Camera* camera, Vector2f playerPosition);

// 更新摄像机位置
//void updateCameraPosition(Camera* camera, Vector2f targetPosition, float deltaTime);

// 处理鼠标消息
void Camera_HandleMouseMessage(Camera* camera, ExMessage* msg, double deltatime);

#endif //STARSECTOR_NANO_CAMERA_H
