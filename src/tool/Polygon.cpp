#include <cmath>
#include "Polygon.h"
#include <cJSON.h> // 需要引入cJSON库
#include <cstdio>


HitPolygonCache* g_polygon_cache_head = NULL;

// 创建一个新多边形
HitPolygon* CreatePolygon(int vertex_count, const Vector2f* vertices) {
    HitPolygon* polygon = (HitPolygon*)malloc(sizeof(HitPolygon));
    polygon->vertex_count = vertex_count;
    polygon->vertices = (Vector2f*)malloc(vertex_count * sizeof(Vector2f));
    memcpy(polygon->vertices, vertices, vertex_count * sizeof(Vector2f));
    return polygon;
}

// 销毁多边形
void DestroyPolygon(HitPolygon* polygon) {
    free(polygon->vertices);
    free(polygon);
}

// 递归释放链表内存
void ClearPolygonCache() {
    HitPolygonCache* current = g_polygon_cache_head;
    while (current) {
        HitPolygonCache* next = current->next;
        DestroyPolygon(current->polygon);
        free(current);
        current = next;
    }
    g_polygon_cache_head = NULL;
}

// 从缓存获取多边形（私有函数）
static HitPolygon* GetCachedPolygon(const char* id_name) {
    HitPolygonCache* current = g_polygon_cache_head;
    while (current) {
        if (strcmp(current->id_name, id_name) == 0) {
            return current->polygon;
        }
        current = current->next;
    }
    return NULL;
}

// 加载JSON并创建多边形
HitPolygon* LoadPolygonFromJSON(const char* id_name, ResourceType resourceType) {
    // 先检查缓存
    if (HitPolygon* cached = GetCachedPolygon(id_name)) {
        return cached;
    }

    // 构建文件路径
    char filename[256];
    switch(resourceType) {
        case SHIP:
            snprintf(filename, sizeof(filename), "assets/data/ship/%s.json", id_name);
            break;
        case BULLET:
            snprintf(filename, sizeof(filename), "assets/data/bullet/%s.json", id_name);
            break;
        default:
            // 默认情况下使用 ship 路径，或者可以根据需求修改
            snprintf(filename, sizeof(filename), "assets/data/ship/%s.json", id_name);
            break;
    }

    // 读取文件
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Hitbox file %s not found\n", filename);
        return NULL;
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
        return NULL;
    }

    // 读取顶点数组
    cJSON* vertices = cJSON_GetObjectItem(root, "vertices");
    if (!vertices || !cJSON_IsArray(vertices)) {
        cJSON_Delete(root);
        return NULL;
    }

    int vertex_count = cJSON_GetArraySize(vertices);
    Vector2f* points = (Vector2f*)malloc(vertex_count * sizeof(Vector2f));

    for (int i = 0; i < vertex_count; i++) {
        cJSON* item = cJSON_GetArrayItem(vertices, i);
        points[i].x = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "x"));
        points[i].y = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "y"));
    }

    // 创建新缓存节点
    HitPolygonCache* new_cache = (HitPolygonCache*)malloc(sizeof(HitPolygonCache));
    strncpy(new_cache->id_name, id_name, sizeof(new_cache->id_name));
    new_cache->polygon = CreatePolygon(vertex_count, points);
    new_cache->next = g_polygon_cache_head;
    g_polygon_cache_head = new_cache;

    free(points);
    cJSON_Delete(root);
    return new_cache->polygon;
}

// 新增函数：计算变换后的顶点坐标
void Polygon_GetTransformedVertices(const HitPolygon* polygon, const Vector2f* position, Vector2f image_center_offset, float angle, Vector2f* result) {
    for (int i = 0; i < polygon->vertex_count; i++) {
        Vector2f rotated;
        Vector2f centered_vertex;
        Vector2f_subtract(&polygon->vertices[i], &image_center_offset, &centered_vertex);
        Vector2f_rotate(&centered_vertex, angle, &rotated);
        Vector2f_add(&rotated, position, &result[i]);
    }
}

static void get_axes(const Vector2f* vertices, int count, Vector2f* axes) {
    for (int i = 0; i < count; i++) {
        Vector2f current = vertices[i];
        Vector2f next = vertices[(i+1)%count];
        Vector2f edge = {next.x - current.x, next.y - current.y};
        Vector2f normal = {-edge.y, edge.x};
        Vector2f_normalize(&normal);
        axes[i] = normal;
    }
}

static void project_polygon(const Vector2f* vertices, int count,
                            const Vector2f* axis,
                            float* min, float* max) {
    *min = INFINITY;
    *max = -INFINITY;
    for (int i = 0; i < count; i++) {
        float proj = Vector2f_dot(&vertices[i], axis);
        *min = fminf(*min, proj);
        *max = fmaxf(*max, proj);
    }
}

static Vector2f calculate_centroid(const Vector2f* vertices, int count) {
    Vector2f centroid = {0};
    for (int i = 0; i < count; i++) {
        centroid.x += vertices[i].x;
        centroid.y += vertices[i].y;
    }
    centroid.x /= count;
    centroid.y /= count;
    return centroid;
}

bool Polygon_CollisionSAT(const HitPolygon* a, const HitPolygon* b, Vector2f* a_verts, Vector2f* b_verts,
                          Vector2f* collision_point, Vector2f* separation_vector) {
    const int a_count = a->vertex_count;
    const int b_count = b->vertex_count;

    // 生成分离轴
    Vector2f axes[64];
    int axis_count = 0;

    // 获取多边形A的轴
    get_axes(a_verts, a_count, axes);
    axis_count += a_count;

    // 获取多边形B的轴
    get_axes(b_verts, b_count, axes + axis_count);
    axis_count += b_count;

    float min_overlap = INFINITY;
    Vector2f min_axis = {0};
    Vector2f centroid_a = calculate_centroid(a_verts, a_count);
    Vector2f centroid_b = calculate_centroid(b_verts, b_count);

    // 检测所有分离轴
    for (int i = 0; i < axis_count; i++) {
        Vector2f axis = axes[i];
        Vector2f_normalize(&axis);

        // 投影计算
        float a_min, a_max, b_min, b_max;
        project_polygon(a_verts, a_count, &axis, &a_min, &a_max);
        project_polygon(b_verts, b_count, &axis, &b_min, &b_max);

        // 分离轴检测
        if (a_max < b_min || b_max < a_min) {
            return false;
        }

        // 计算重叠量
        float overlap = fminf(a_max - b_min, b_max - a_min);
        if (overlap < min_overlap) {
            min_overlap = overlap;
            min_axis = axis;

            // 确定分离方向
            Vector2f delta_centroid = {centroid_b.x - centroid_a.x, centroid_b.y - centroid_a.y};
            if (Vector2f_dot(&delta_centroid, &axis) < 0) {
                min_axis.x = -min_axis.x;
                min_axis.y = -min_axis.y;
            }
        }
    }

    // 计算碰撞结果
    if (collision_point) {
        collision_point->x = (centroid_a.x + centroid_b.x) * 0.5f;
        collision_point->y = (centroid_a.y + centroid_b.y) * 0.5f;
    }

    if (separation_vector) {
        separation_vector->x = min_axis.x * min_overlap;
        separation_vector->y = min_axis.y * min_overlap;
    }

    return true;
}
