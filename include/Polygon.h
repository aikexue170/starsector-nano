#ifndef STARSECTOR_NANO_POLYGON_H
#define STARSECTOR_NANO_POLYGON_H

#include <cstdlib>
#include <cstring>
#include "Vector2f.h"

// 多边形结构体
typedef struct HitPolygon{
    int vertex_count; // 顶点数量
    Vector2f* vertices; // 顶点数组
} HitPolygon;

// 读取器的类型选择
typedef enum ResourceType {
    SHIP,
    BULLET,
} ResourceType;

// 碰撞箱缓存节点
typedef struct HitPolygonCache {
    char id_name[64];
    HitPolygon* polygon;
    struct HitPolygonCache* next;
} HitPolygonCache;

// 全局链表头指针
extern HitPolygonCache* g_polygon_cache_head;

// 新增接口
HitPolygon* LoadPolygonFromJSON(const char* id_name, ResourceType resourceType);
void ClearPolygonCache();

// 创建一个新多边形
HitPolygon* CreatePolygon(int vertex_count, const Vector2f* vertices);
// 销毁多边形
void DestroyPolygon(HitPolygon* polygon);
// 计算变换后的顶点坐标
void Polygon_GetTransformedVertices(const HitPolygon* polygon, const Vector2f* position, Vector2f image_center_offset, float angle, Vector2f* result);
// 碰撞检测函数
bool Polygon_CollisionSAT(const HitPolygon* a, const HitPolygon* b,
                          Vector2f* a_verts, Vector2f* b_verts,
                          Vector2f* collision_point, Vector2f* separation_vector);
#endif //STARSECTOR_NANO_POLYGON_H
