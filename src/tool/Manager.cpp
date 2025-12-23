#include "Manager.h"
#include "ImageProcessing.h"


Vector2f MousePosition;
Vector2f PlayerPosition;

int BackgroundID = 0;
int UIID = 10;
int EntityID = 100;
int EffectID = 10000;
int WeaponID = 100000;

float FlushingAlpha = 0;

// 创建新的渲染节点
static RenderNode* RenderManager_CreateRenderNode(IMAGE* image, int x, int y, float angle, int isUI) {
    RenderNode* newNode = (RenderNode*)malloc(sizeof(RenderNode));
    if (newNode != NULL) {
        newNode->image = image;
        newNode->x = x;
        newNode->y = y;
        newNode->angle = angle;
        newNode->isUI = isUI;
        newNode->next = NULL;
    }
    return newNode;
}

// 在RenderManager中添加ID链表释放功能
void RenderManager_DeleteIdNode(RenderManager* rm, int id) {
    if(!rm || id < 0) return;

    IdNode* current = rm->idList; // 假设idList是ID链表的头指针
    IdNode* prev = NULL;

    while(current != NULL) {
        if(current->id == id) {
            if(prev == NULL) {
                rm->idList = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// 删除指定 ID 的节点
bool RenderManager_DeleteNode(RenderManager* rm, int id) {
    RenderManager_DeleteIdNode(rm, id);
    // 遍历所有链表，找到对应 ID 的节点并删除
    RenderNode** currentList = NULL;

    if ((0 <= id) && (id < 10)) {
        currentList = &rm->backgrounds; // 背景链表
    } else if ((10 <= id) && (id < 100)) {
        currentList = &rm->uiElements; // UI 元素链表
    } else if ((100 <= id) && (id < 10000)) {
        currentList = &rm->entities; // 实体链表
    } else if ((10000 <= id) && (id < 100000)) {
        currentList = &rm->effects; // 特效链表
    } else if ((100000 <= id) && (id < 1000000)) {
        currentList = &rm->weapons; // 武器链表
    } else {
        return false; // 无效的 ID 范围
    }

    RenderNode* current = *currentList;
    RenderNode* prev = NULL;

    // 遍历链表，查找目标节点
    while (current != NULL) {
        if (current->id == id) {
            // 如果找到了目标节点
            if (prev == NULL) {
                // 如果是头节点
                *currentList = current->next;
            } else {
                // 如果不是头节点
                prev->next = current->next;
            }
            free(current); // 释放节点内存
            return true; // 删除成功
        }
        prev = current;
        current = current->next;
    }

    return false; // 没有找到对应的 ID
}

// 创建ID节点
static IdNode* RenderManager_CreateIdNode(bool IsBackground, bool IsEntity, bool IsUI, bool IsWeapon, bool IsEffect, int id) {
    IdNode* newNode = (IdNode*)malloc(sizeof(IdNode));
    if (newNode != NULL) {
        newNode->IsBackground = IsBackground;
        newNode->IsEntity = IsEntity;
        newNode->IsUI = IsUI;
        newNode->IsWeapon = IsWeapon;
        newNode->IsEffect = IsEffect;
        newNode->id = id;
        newNode->next = NULL;
    }
    return newNode;
}

// 插入ID节点
static bool RenderManager_InsertIdNode(RenderManager* rm, bool IsBackground, bool IsEntity, bool IsUI, bool IsWeapon,bool IsEffect,  int id) {
    IdNode* newIdNode = RenderManager_CreateIdNode(IsBackground, IsEntity, IsUI, IsWeapon, IsEffect, id);
    if (newIdNode != NULL) {
        newIdNode->next = rm->idList;
        rm->idList = newIdNode;
        return true;
    }
    return false;
}

// 初始化渲染器
void RenderManager_Init(RenderManager* rm) {
    rm->backgrounds = NULL;
    rm->entities = NULL;
    rm->uiElements = NULL;
    rm->effects = NULL;
    rm->weapons = NULL;
    rm->idList = NULL;
    rm->shakeAmplitude = 0;
    rm->isShaking = 0;
}
// 应用世界坐标变换
static void RenderManager_ApplyTransform(HDC dc, float angle, int x, int y) {
    XFORM transform;
    transform.eM11 = cosf(angle * M_PI / 180); // 旋转矩阵的 cos 角度
    transform.eM12 = sinf(angle * M_PI / 180); // 旋转矩阵的 sin 角度
    transform.eM21 = -sinf(angle * M_PI / 180);
    transform.eM22 = cosf(angle * M_PI / 180);
    transform.eDx = x; // 平移 x
    transform.eDy = y; // 平移 y

    SetWorldTransform(dc, &transform); // 设置世界坐标变换

}

// 恢复默认世界坐标变换
static void RenderManager_ResetTransform(HDC dc) {
    XFORM identity;
    identity.eM11 = 1.0f; identity.eM12 = 0.0f;
    identity.eM21 = 0.0f; identity.eM22 = 1.0f;
    identity.eDx = 0.0f; identity.eDy = 0.0f;

    SetWorldTransform(dc, &identity); // 恢复单位矩阵
}

// 绘制图片
void RenderManager_Draw(int x, int y, IMAGE* image, float angle){
    if (!image) return; // 防止空指针

    HDC dc = GetImageHDC(); // 获取当前绘图设备上下文
    int width = image->getwidth();
    int height = image->getheight();

    // 应用旋转和平移变换
    RenderManager_ApplyTransform(dc, angle, x, y);

    // 设置绘制位置为图片中心点
    int dx = -width / 2;
    int dy = -height / 2;
    AlphaBlend(dc, dx, dy, width, height,
               GetImageHDC(image), 0, 0, width, height, { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA });

    // 恢复默认世界坐标变换
    RenderManager_ResetTransform(dc);
}

// 绘制图片
void RenderManager_Draw(int x, int y, IMAGE* image){
    if (!image) return; // 防止空指针

    HDC dc = GetImageHDC(); // 获取当前绘图设备上下文
    int width = image->getwidth();
    int height = image->getheight();

    // 设置绘制位置为图片中心点
    int dx = -width / 2;
    int dy = -height / 2;
    AlphaBlend(dc, dx, dy, width, height,
               GetImageHDC(image), 0, 0, width, height, { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA });


}

// 添加背景
int RenderManager_AddBackground(RenderManager* rm, IMAGE* image) {
    if (image) {
        RenderNode* newNode = RenderManager_CreateRenderNode(image, 0, 0, 0.0f, 0);
        newNode->id = BackgroundID++;
        newNode->next = rm->backgrounds;
        rm->backgrounds = newNode;

        if(RenderManager_InsertIdNode(rm, true, false, false, false, false, BackgroundID - 1)){
            return BackgroundID - 1;
        }
    }
    return -1;
}

// 添加实体
int RenderManager_AddEntity(RenderManager* rm, IMAGE* image, int x, int y, float angle) {
    if (image) {
        RenderNode* newNode = RenderManager_CreateRenderNode(image, x, y, angle, 0);
        newNode->id = EntityID++;
        newNode->next = rm->entities;
        rm->entities = newNode;

        if(RenderManager_InsertIdNode(rm, false, true, false, false, false, EntityID - 1)){
            return EntityID - 1;
        }
    }
    return -2;
}

// 添加 UI 元素
int RenderManager_AddUIElement(RenderManager* rm, IMAGE* image, int x, int y) {
    if (image) {
        RenderNode* newNode = RenderManager_CreateRenderNode(image, x, y, 0.0f, 1);
        newNode->id = UIID++;
        newNode->next = rm->uiElements;
        rm->uiElements = newNode;

        if(RenderManager_InsertIdNode(rm, false, false, true, false, false, UIID - 1)){
            return UIID - 1;
        }
    }
    return -3;
}

// 添加武器元素
int RenderManager_AddWeapon(RenderManager* rm, IMAGE* image, int x, int y, float angle) {
    if (image) {
        RenderNode* newNode = RenderManager_CreateRenderNode(image, x, y, angle, 0);
        newNode->id = WeaponID++;
        newNode->next = rm->weapons;
        rm->weapons = newNode;

        if(RenderManager_InsertIdNode(rm, false, false, false, true, false, WeaponID - 1)){
            return WeaponID - 1;
        }
    }
    return -4;
}

// 添加特效元素
int RenderManager_AddEffect(RenderManager* rm, IMAGE* image, int x, int y, float angle) {
    if (image) {
        RenderNode* newNode = RenderManager_CreateRenderNode(image, x, y, angle, 0);
        newNode->id = EffectID++;
        newNode->next = rm->effects;
        rm->effects = newNode;

        if(RenderManager_InsertIdNode(rm, false, false, false, false, true, EffectID - 1)){
            return EffectID - 1;
        }
    }
    return -6;
}

void RenderManager_MouseUpdate(RenderManager* rm, Vector2f position){
    MousePosition = position;
}

void RenderManager_PlayerUpdate(RenderManager* rm, Vector2f position){
    PlayerPosition = position;
}

bool RenderManager_Update(RenderManager* rm, int id, int x, int y, float angle, IMAGE* image){

    RenderNode* EntityCurrent = rm->entities;
    RenderNode* UIElementCurrent = rm->uiElements;
    RenderNode* BackgroundCurrent = rm->backgrounds;
    RenderNode* EffectCurrent = rm->effects;
    RenderNode* WeaponCurrent = rm->weapons;

    if ((0 <= id) && (id < 10)) {
        while (BackgroundCurrent != NULL) {
            if (BackgroundCurrent->id == id) {
                BackgroundCurrent->x = x;
                BackgroundCurrent->y = y;
                BackgroundCurrent->angle = angle;
                BackgroundCurrent->image = image;
                return true;
            }
            BackgroundCurrent = BackgroundCurrent->next;
        }
    } else if ((10 <= id) && (id < 100)) {
        while (UIElementCurrent != NULL) {
            if (UIElementCurrent->id == id) {
                UIElementCurrent->x = x;
                UIElementCurrent->y = y;
                UIElementCurrent->angle = angle;
                UIElementCurrent->image = image;
                return true;
            }
            UIElementCurrent = UIElementCurrent->next;
        }
    } else if ((100 <= id) && (id < 10000)) {
        while (EntityCurrent != NULL) {
            if (EntityCurrent->id == id) {
                EntityCurrent->x = x;
                EntityCurrent->y = y;
                EntityCurrent->angle = angle;
                EntityCurrent->image = image;
                return true;
            }
            EntityCurrent = EntityCurrent->next;
        }
    } else if ((10000 <= id) && (id < 100000)) {
        while (EffectCurrent != NULL) {
            if (EffectCurrent->id == id) {
                EffectCurrent->x = x;
                EffectCurrent->y = y;
                EffectCurrent->angle = angle;
                EffectCurrent->image = image;
                return true;
            }
            EffectCurrent = EffectCurrent->next;
        }
    } else if ((100000 <= id) && (id < 1000000)) {
        while (WeaponCurrent != NULL) {
            if (WeaponCurrent->id == id) {
                WeaponCurrent->x = x;
                WeaponCurrent->y = y;
                WeaponCurrent->angle = angle;
                WeaponCurrent->image = image;
                return true;
            }
            WeaponCurrent = WeaponCurrent->next;
        }
    }
    return false;
}
void RenderManager_Render(RenderManager* rm) {
    static IMAGE ScreenFlushImage = ResourceManager_LoadImage("assets/graphic/fx/white.png");
    static IMAGE ScreenFlushImage_alpha;
    static IMAGE ScreenFlushImage_alpha_blend;
    static IMAGE empty = ResourceManager_LoadImage("assets/graphic/fx/white_base.png");
    int offsetX = 0, offsetY = 0;
    if (rm->isShaking) {
        offsetX = rand() % (rm->shakeAmplitude * 2) - rm->shakeAmplitude;
        offsetY = rand() % (rm->shakeAmplitude * 2) - rm->shakeAmplitude;
    }

    // 计算以玩家为中心的偏移量
    int screenCenterX = 1280 / 2;
    int screenCenterY = 720 / 2;
    int playerOffsetX = screenCenterX - PlayerPosition.x;
    int playerOffsetY = screenCenterY - PlayerPosition.y;

    // 绘制背景（现在基于玩家位置和鼠标位置偏移）
    RenderNode* current = rm->backgrounds;
    while (current != NULL) {
        if (current->image) {
            // 背景偏移 = 玩家偏移 + 鼠标偏移
            RenderManager_Draw(offsetX + current->x + playerOffsetX - MousePosition.x * BACKGROUND_OFFSET,
                               offsetY + current->y + playerOffsetY - MousePosition.y * BACKGROUND_OFFSET,
                               current->image, 0);
        }
        current = current->next;
    }

    // 绘制实体（从后往前渲染）
    RenderNode* lastEntity = NULL;
    current = rm->entities;
    // 找到最后一个实体节点
    while (current != NULL) {
        lastEntity = current;
        current = current->next;
    }
    // 从最后一个节点开始向前渲染
    current = lastEntity;
    while (current != NULL) {
        if (current->image) {
            // 实体偏移 = 玩家偏移 + 鼠标偏移
            RenderManager_Draw(offsetX + current->x + playerOffsetX - MousePosition.x * ENTITY_EFFECT_OFFSET,
                               offsetY + current->y + playerOffsetY - MousePosition.y * ENTITY_EFFECT_OFFSET,
                               current->image,
                               current->angle);
        }
        RenderNode* prev = NULL;
        RenderNode* temp = rm->entities;
        while (temp != NULL && temp->next != current) {
            temp = temp->next;
        }
        if (temp != NULL) { // 如果找到了，则temp为前一个节点
            prev = temp;
        }
        current = prev; // 向前移动到前一个节点
    }

    // 绘制 UI 元素（保持不变，不需要偏移）
    current = rm->uiElements;
    while (current != NULL) {
        if (current->image) {
            RenderManager_Draw(current->x, current->y, current->image, 0);
        }
        current = current->next;
    }

    // 绘制武器元素（玩家偏移 + 鼠标偏移）
    current = rm->weapons;
    while (current != NULL) {
        if (current->image) {
            RenderManager_Draw(offsetX + current->x + playerOffsetX - MousePosition.x * ENTITY_EFFECT_OFFSET,
                               offsetY + current->y + playerOffsetY - MousePosition.y * ENTITY_EFFECT_OFFSET,
                               current->image,
                               current->angle);
        }
        current = current->next;
    }

    // 绘制特效元素（玩家偏移 + 鼠标偏移）
    current = rm->effects;
    while (current != NULL) {
        if (current->image) {
            RenderManager_Draw(offsetX + current->x + playerOffsetX - MousePosition.x * ENTITY_EFFECT_OFFSET,
                               offsetY + current->y + playerOffsetY - MousePosition.y * ENTITY_EFFECT_OFFSET,
                               current->image,
                               current->angle);
        }
        current = current->next;
    }

    if (rm->shakeAmplitude > 0) {
        rm->shakeAmplitude -= 1;
    }
    if (rm->shakeAmplitude == 0) {
        rm->isShaking = 0;
    }

    if (FlushingAlpha > 0){
        if (FlushingAlpha > 1){
            RenderManager_Draw(640, 360, &ScreenFlushImage, 0);
            FlushingAlpha -= 0.01;
        }else{
            ImageProcessing_AdjustImageTransparency(&ScreenFlushImage_alpha, &ScreenFlushImage, FlushingAlpha);
            ImageProcessing_BlendImages(&ScreenFlushImage_alpha_blend, &ScreenFlushImage_alpha, &empty);
            RenderManager_Draw(640, 360, &ScreenFlushImage_alpha_blend, 0);
            FlushingAlpha -= 0.01;
        }
    }
}

// 震动方法
void RenderManager_ShakeScreen(RenderManager* rm, int amplitude) {
    rm->shakeAmplitude = amplitude;
    rm->isShaking = 1;
}

// 清空所有对象并释放内存
void RenderManager_ClearAllObjects(RenderManager* rm) {
    // 清空背景链表
    RenderNode* current = rm->backgrounds;
    while (current != NULL) {
        RenderNode* temp = current;
        current = current->next;
        free(temp);
    }
    rm->backgrounds = NULL;

    // 清空实体链表
    current = rm->entities;
    while (current != NULL) {
        RenderNode* temp = current;
        current = current->next;
        free(temp);
    }
    rm->entities = NULL;

    // 清空 UI 元素链表
    current = rm->uiElements;
    while (current != NULL) {
        RenderNode* temp = current;
        current = current->next;
        free(temp);
    }
    rm->uiElements = NULL;

    // 清空 ID 链表
    IdNode* IdCurrent = rm->idList;
    while (IdCurrent != NULL) {
        IdNode* temp = IdCurrent;
        IdCurrent = IdCurrent->next;
        free(temp);
    }
    rm->idList = NULL;

    // 清空武器链表
    current = rm->weapons;
    while (current != NULL) {
        RenderNode* temp = current;
        current = current->next;
        free(temp);
    }

    // 清空特效链表
    current = rm->effects;
    while (current != NULL) {
        RenderNode* temp = current;
        current = current->next;
        free(temp);
    }
    rm->effects = NULL;
}

// 创建新的资源节点
static ResourceNode* CreateResourceNode(const char* category, const char* name, const char* path) {
    ResourceNode* newNode = (ResourceNode*)malloc(sizeof(ResourceNode));
    if (newNode != NULL) {
        strncpy(newNode->category, category, MAX_CATEGORY_LENGTH);
        strncpy(newNode->name, name, MAX_NAME_LENGTH);
        strncpy(newNode->path, path, MAX_PATH_LENGTH);
        newNode->next = NULL;
    }
    return newNode;
}

// 初始化资源管理器
void ResourceManager_Init(ResourceManager* rm) {
    rm->head = NULL;
}

// 注册资源
void ResourceManager_RegisterResource(ResourceManager* rm, const char* category, const char* name, const char* path) {
    ResourceNode* newNode = CreateResourceNode(category, name, path);
    if (newNode != NULL) {
        newNode->next = rm->head;
        rm->head = newNode;
        printf("Registered resource: Category=%s, Name=%s, Path=%s\n", category, name, path);
    } else {
        printf("Failed to register resource: Memory allocation error.\n");
    }
}

// JSON注册
void ResourceManager_RegisterAllFromJSON(ResourceManager* rm) {
    // 打开JSON文件
    FILE* file = fopen("assets/data/register.json", "rb");
    if (!file) {
        perror("[ResourceManager] Failed to open register.json");
        return;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 读取文件内容
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        perror("[ResourceManager] Memory allocation failed");
        return;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    fclose(file);

    // 解析JSON
    cJSON* root = cJSON_Parse(buffer);
    if (!root) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            fprintf(stderr, "[ResourceManager] JSON parse error: %s\n", error_ptr);
        }
        free(buffer);
        return;
    }

    // 遍历所有资源类别
    cJSON* category = NULL;
    cJSON_ArrayForEach(category, root) {
        const char* category_name = category->string;

        // 遍历类别下的所有资源
        cJSON* item = NULL;
        cJSON_ArrayForEach(item, category) {
            const char* resource_name = item->string;
            const char* path = item->valuestring;

            // 注册资源到管理器
            ResourceManager_RegisterResource(
                    rm,
                    category_name,
                    resource_name,
                    path
            );
        }
    }

    // 清理资源
    cJSON_Delete(root);
    free(buffer);
}

// 获取资源路径
const char* ResourceManager_GetResourcePath(ResourceManager* rm, const char* category, const char* name) {
    ResourceNode* current = rm->head;
    while (current != NULL) {
        if (strcmp(current->category, category) == 0 && strcmp(current->name, name) == 0) {
            return current->path;
        }
        current = current->next;
    }
    printf("Resource not found: Category=%s, Name=%s\n", category, name);
    return NULL;
}

// 封装的 LoadImage 函数
IMAGE ResourceManager_LoadImage(const char* path) {
    IMAGE image = NULL;  // 初始化为NULL
    // 传递指针地址以正确加载图像
    loadimage(&image, path, 0, 0, false);
    printf("成功加载图片: Path=%s\n", path);
    return image;
}

// 获取 IMAGE
void ResourceManager_GetIMAGE(ResourceManager* rm, IMAGE* image, const char* category, const char* name) {
    if (ResourceManager_GetResourcePath(rm, category, name) != NULL) {
        loadimage(image, ResourceManager_GetResourcePath(rm, category, name), 0, 0, false);
    }
}

// 删除资源并释放内存
void ResourceManager_RemoveResource(ResourceManager* rm, const char* category, const char* name) {
    ResourceNode* current = rm->head;
    ResourceNode* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->category, category) == 0 && strcmp(current->name, name) == 0) {
            if (prev == NULL) {
                rm->head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            printf("Removed resource: Category=%s, Name=%s\n", category, name);
            return;
        }
        prev = current;
        current = current->next;
    }
    printf("Resource not found: Category=%s, Name=%s\n", category, name);
}

void RenderManager_UpdateCameraScreenPosition(Camera* camera, float offsetFactor) {
    if (camera == NULL) {
        fprintf(stderr, "Error: Camera is NULL\n");
        return;
    }

    Vector2f mouseWorldPosition = camera->position;
    // 计算鼠标相对于摄像机中心的偏移量
    float mouseOffsetX = mouseWorldPosition.x;
    float mouseOffsetY = mouseWorldPosition.y;

    // 计算摄像机的屏幕位置（考虑视差偏移）
    // 屏幕中心是 (640, 360)，所以摄像机位置应该补偿偏移
    camera->screen_position.x = 640 + mouseOffsetX * offsetFactor;
    camera->screen_position.y = 360 + mouseOffsetY * offsetFactor;
}

// 清空所有资源并释放内存
void ResourceManager_Clear(ResourceManager* rm) {
    ResourceNode* current = rm->head;
    ResourceNode* temp;

    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);
    }
    rm->head = NULL;
    printf("All resources cleared.\n");
}

void RenderManager_ScreenFlush(){
    FlushingAlpha = 1.2f;
}