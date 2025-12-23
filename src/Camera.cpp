#include <cstdio>
#include <iostream>
#include "Camera.h"

/*
// 获取鼠标位置
Vector2f Camera_GetMousePosition() {
    static Vector2f lastPos = {0.0f, 0.0f}; // 初始化静态变量保存上一次的位置
    ExMessage msg;
    if (peekmessage(&msg, EX_MOUSE)) { // 检查是否有新的鼠标消息
        Vector2f mousePos;
        // 调整x和y坐标，使其基于屏幕中心
        mousePos.x = (float)(msg.x - 1280 / 2);
        mousePos.y = (float)(msg.y - 720 / 2);
        lastPos = mousePos; // 更新最后的位置为当前鼠标位置
    }
    return lastPos; // 返回最后的有效位置
}
*/

void Camera_UpdatePlayerPosition(Camera* camera, Vector2f playerPosition){
    camera->player_position = playerPosition;
}

// 获取鼠标位置并调整到相对于屏幕中心的位置
Vector2f Camera_GetMousePosition() {
    POINT cursorPos;
    GetCursorPos(&cursorPos); // 获取当前鼠标的屏幕坐标

    // 游戏窗口句柄
    HWND hWnd = GetForegroundWindow(); // 或者用其他方式获取你的游戏窗口句柄
    ScreenToClient(hWnd, &cursorPos); // 将屏幕坐标转换为窗口客户区坐标

    Vector2f mousePos;
    // 调整x和y坐标，使其基于屏幕中心
    mousePos.x = (float)(cursorPos.x - 1280 / 2);
    mousePos.y = (float)(cursorPos.y - 720 / 2);

    return mousePos;
}

// 初始化摄像机
void Camera_Init(Camera* camera, float startX, float startY) {
    camera->position.x = startX;
    camera->position.y = startY;
}

// 更新摄像机位置
void updateCameraPosition(Camera* camera, Vector2f targetPosition, float deltaTime, SmoothMode mode) {
    switch (mode) {
        case SMOOTH_EXPONENTIAL: {
            float alpha = 0.1f; // 衰减因子
            camera->position.x = MathTool_ExponentialSmoothing(targetPosition.x, camera->position.x, alpha);
            camera->position.y = MathTool_ExponentialSmoothing(targetPosition.y, camera->position.y, alpha);
            break;
        }
        case SMOOTH_SIGMOID: {
            // 假设k为控制Sigmoid曲线的陡峭程度
            float k = 1.0f;
            camera->position.x = MathTool_Sigmoid(targetPosition.x - camera->position.x, k) + camera->position.x;
            camera->position.y = MathTool_Sigmoid(targetPosition.y - camera->position.y, k) + camera->position.y;
            break;
        }
        case SMOOTH_SMOOTHSTEP: {
            // 使用Smoothstep函数需要指定边缘值，这里简单处理为固定范围
            float edge0 = -1000.0f, edge1 = 1000.0f;
            camera->position.x = MathTool_Smoothstep(edge0, edge1, targetPosition.x - camera->position.x) * (edge1 - edge0) + edge0 + camera->position.x;
            camera->position.y = MathTool_Smoothstep(edge0, edge1, targetPosition.y - camera->position.y) * (edge1 - edge0) + edge0 + camera->position.y;
            break;
        }
        case SMOOTH_EASEINOUTQUAD: {
            // EaseInOutQuad函数通常用于[0,1]范围内的t值，因此需要进行转换
            float tX = (targetPosition.x - camera->position.x) / 1000.0f + 0.5f; // 简单调整范围
            float tY = (targetPosition.y - camera->position.y) / 1000.0f + 0.5f;
            tX = MathTool_EaseInOutQuad(tX);
            tY = MathTool_EaseInOutQuad(tY);
            camera->position.x += (tX - 0.5f) * 1000.0f;
            camera->position.y += (tY - 0.5f) * 1000.0f;
            break;
        }
        default:
            // 默认使用指数平滑
            float alpha = 0.1f;
            camera->position.x = MathTool_ExponentialSmoothing(targetPosition.x, camera->position.x, alpha);
            camera->position.y = MathTool_ExponentialSmoothing(targetPosition.y, camera->position.y, alpha);
    }
}

// 处理鼠标消息
void Camera_HandleMouseMessage(Camera* camera, ExMessage* msg, double deltatime) {
    switch (msg->message) {

        case WM_LBUTTONDOWN:
            printf("Left mouse button down at (%d, %d)\n", msg->x, msg->y);
            break;
        case WM_RBUTTONDOWN:
            printf("Right mouse button down at (%d, %d)\n", msg->x, msg->y);
            break;
    }
    Vector2f targetPos = Camera_GetMousePosition();
    updateCameraPosition(camera, targetPos, deltatime, SMOOTH_EXPONENTIAL);
}