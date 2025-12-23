
#ifndef VECTOR2F_H
#define VECTOR2F_H

typedef struct {
    float x;
    float y;
} Vector2f;

void Vector2f_init(Vector2f *vec, float x, float y);
float Vector2f_length(const Vector2f *vec);
void Vector2f_add(const Vector2f *vec1, const Vector2f *vec2, Vector2f *result);
void Vector2f_subtract(const Vector2f *vec1, const Vector2f *vec2, Vector2f *result);
void Vector2f_scale(const Vector2f *vec, float scale, Vector2f *result);
void Vector2f_rotate(const Vector2f* vec, float degrees, Vector2f* result);
float Vector2f_dot(const Vector2f* a, const Vector2f* b);
void Vector2f_normalize(Vector2f* vec);
#endif
