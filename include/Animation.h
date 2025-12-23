#ifndef STARSECTOR_NANO_ANIMATION_H
#define STARSECTOR_NANO_ANIMATION_H

#include "easyx.h"
#include <string.h>
#include <cstdio>

// 定义 Atlas 结构体
typedef struct {
    IMAGE frames[24];      // 存储 24 帧图像
    int frame;             // 当前播放到第几帧
    char name[64];         // 动画的名字
} Atlas;

// 初始化 Atlas 结构体
void Atlas_Init(Atlas* atlas, const char* name) {
    memset(atlas->frames, 0, sizeof(IMAGE) * 24); // 初始化图像数组
    atlas->frame = 0;                             // 默认从第 0 帧开始
    strncpy(atlas->name, name, sizeof(atlas->name) - 1); // 设置名字
    atlas->name[sizeof(atlas->name) - 1] = '\0';  // 确保字符串以 '\0' 结尾
}

// 加载帧图像
void Atlas_LoadFrame(Atlas* atlas, int index, const char* imagePath) {
    if (index < 0 || index >= 24) {
        printf("Error: Frame index %d out of range!\n", index);
        return;
    }
    loadimage(&atlas->frames[index], imagePath); // 使用 EasyX 的 loadimage 函数加载图像
    printf("Loaded frame %d for animation '%s'\n", index, atlas->name);
}

// 更新到下一帧
void Atlas_NextFrame(Atlas* atlas) {
    atlas->frame = (atlas->frame + 1) % 24; // 循环播放
    printf("Animation '%s' advanced to frame %d\n", atlas->name, atlas->frame);
}

// 获取当前帧图像
IMAGE* Atlas_GetCurrentFrame(Atlas* atlas) {
    return &atlas->frames[atlas->frame];
}

#endif //STARSECTOR_NANO_ANIMATION_H
