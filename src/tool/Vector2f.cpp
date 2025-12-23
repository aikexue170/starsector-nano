#include <math.h>
#include "Vector2f.h"


void Vector2f_init(Vector2f *vec, float x, float y) {
    vec->x = x;
    vec->y = y;
}

float Vector2f_length(const Vector2f *vec) {
    return sqrt(vec->x * vec->x + vec->y * vec->y);
}

void Vector2f_add(const Vector2f *vec1, const Vector2f *vec2, Vector2f *result) {
    result->x = vec1->x + vec2->x;
    result->y = vec1->y + vec2->y;
}

void Vector2f_subtract(const Vector2f *vec1, const Vector2f *vec2, Vector2f *result) {
    result->x = vec1->x - vec2->x;
    result->y = vec1->y - vec2->y;
}

void Vector2f_scale(const Vector2f *vec, float scale, Vector2f *result) {
    result->x = vec->x * scale;
    result->y = vec->y * scale;
}

void Vector2f_rotate(const Vector2f* vec, float degrees, Vector2f* result) {
    float rad = degrees * (float)(M_PI / 180.0);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    result->x = vec->x * cosA - vec->y * sinA;
    result->y = vec->x * sinA + vec->y * cosA;
}

float Vector2f_dot(const Vector2f* a, const Vector2f* b) {
    return a->x * b->x + a->y * b->y;
}

void Vector2f_normalize(Vector2f* vec) {
    float len = Vector2f_length(vec);
    if (len != 0) {
        vec->x /= len;
        vec->y /= len;
    }
}