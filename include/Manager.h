#ifndef MANAGER_H
#define MANAGER_H

#include "easyx.h"
#include <cmath>
#include <cstdio>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "cJSON.h"
#include "Vector2f.h"
#include "Camera.h"

// 鼠标位置
extern Vector2f MousePosition;
extern Vector2f PlayerPosition;

// ID序号分配
extern int BackgroundID;
extern int UIID;
extern int EntityID;
extern int WeaponID;
extern int EffectID;
extern float FlushingAlpha;

// 资源管理器设置
#define MAX_CATEGORY_LENGTH 128
#define MAX_NAME_LENGTH 128
#define MAX_PATH_LENGTH 256
#define BACKGROUND_OFFSET 0.3
#define ENTITY_EFFECT_OFFSET 1

// 渲染对象链表节点
typedef struct RenderNode {
    int id;                   // ID
    IMAGE* image;            // 图像指针
    float x, y;                // 坐标
    float angle;             // 旋转角度
    int isUI;                // 是否为 UI 元素
    struct RenderNode* next; // 指向下一个渲染对象的指针
} RenderNode;

// ID节点
typedef struct IdNode {
    bool IsBackground;  // 是否为背景
    bool IsEntity;      // 是否为实体
    bool IsUI;          // 是否为UI元素
    bool IsWeapon;       // 是否为武器
    bool IsEffect;       // 是否为特效
    int id;             // ID
    struct IdNode* next;// 指向下一个节点的指针
} IdNode;

// 资源节点
typedef struct ResourceNode {
    char category[MAX_CATEGORY_LENGTH];// 资源类别
    char name[MAX_NAME_LENGTH];        // 资源名称
    char path[MAX_PATH_LENGTH];        // 资源路径
    struct ResourceNode* next;         // 指向下一个资源的指针
} ResourceNode;

// 资源管理器结构体
typedef struct {
    ResourceNode* head;
} ResourceManager;

// 渲染器结构体
typedef struct {
    RenderNode* backgrounds; // 背景链表
    RenderNode* entities;    // 实体链表
    RenderNode* uiElements;  // UI 元素链表
    RenderNode* weapons;     // 武器链表
    RenderNode* effects;     // 特效链表
    IdNode* idList;          // ID链表
    int shakeAmplitude;      // 震动幅度
    int isShaking;           // 是否震动
} RenderManager;


// 函数声明

// 渲染器相关
void RenderManager_Init(RenderManager* rm);                                             // 初始化渲染器
void RenderManager_Draw(int x, int y, IMAGE* image, float angle);                       // 绘制图像
int RenderManager_AddBackground(RenderManager* rm, IMAGE* image);                       // 添加背景
int RenderManager_AddEntity(RenderManager* rm, IMAGE* image, int x, int y, float angle);// 添加实体
int RenderManager_AddUIElement(RenderManager* rm, IMAGE* image, int x, int y);          // 添加UI元素
int RenderManager_AddWeapon(RenderManager* rm, IMAGE* image, int x, int y, float angle);// 添加武器
int RenderManager_AddEffect(RenderManager* rm, IMAGE* image, int x, int y, float angle);// 添加特效
bool RenderManager_Update(RenderManager* rm, int id, int x, int y, float angle, IMAGE* image = NULL);        // 更新状态
void RenderManager_MouseUpdate(RenderManager* rm, Vector2f position);
void RenderManager_PlayerUpdate(RenderManager* rm, Vector2f position);
void RenderManager_Render(RenderManager* rm);                                           // 渲染
void RenderManager_ShakeScreen(RenderManager* rm, int amplitude);                       // 震屏特效
void RenderManager_ScreenFlush();                                                       // 白屏特效
void RenderManager_ClearAllObjects(RenderManager* rm);                                  // 销毁渲染器
void RenderManager_UpdateCameraScreenPosition(Camera* camera, float offsetFactor = ENTITY_EFFECT_OFFSET);
bool RenderManager_DeleteNode(RenderManager* rm, int id);                               // 删除节点
void RenderManager_DeleteIdNode(RenderManager* rm, int id);

// 资源管理器相关
void ResourceManager_Init(ResourceManager* rm);                                                                      // 初始化资源管理器
void ResourceManager_RegisterResource(ResourceManager* rm, const char* category, const char* name, const char* path);// 注册资源
void ResourceManager_RegisterAllFromJSON(ResourceManager* rm);
const char* ResourceManager_GetResourcePath(ResourceManager* rm, const char* category, const char* name);            // 获取资源路径
IMAGE ResourceManager_LoadImage(const char* path);                                                                   // 按路径加载图片
void ResourceManager_GetIMAGE(ResourceManager* rm, IMAGE* image, const char* category, const char* name);                         // 获取图片
void ResourceManager_RemoveResource(ResourceManager* rm, const char* category, const char* name);                    // 移除资源
void ResourceManager_Clear(ResourceManager* rm);                                                                     // 销毁资源管理器

// 所有静态函数（忘了有啥的话就来查查看吧）
//ResourceNode* CreateResourceNode(const char* category, const char* name, const char* path);
//void RenderManager_ApplyTransform(HDC dc, float angle, int x, int y);
//void RenderManager_ResetTransform(HDC dc);
//RenderNode* RenderManager_CreateRenderNode(IMAGE* image, int x, int y, float angle, int isUI);
//IdNode* RenderManager_CreateIdNode(bool IsBackground, bool IsEntity, bool IsUI, int id);
//bool RenderManager_InsertIdNode(RenderManager* rm, bool IsBackground, bool IsEntity, bool IsUI, int id);

#endif //STARSECTOR_NANO_MANAGER_H
