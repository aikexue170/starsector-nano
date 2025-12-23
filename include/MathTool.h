#ifndef STARSECTOR_NANO_MATHTOOL_H
#define STARSECTOR_NANO_MATHTOOL_H
#include <math.h>
typedef enum {
    SMOOTH_EXPONENTIAL,
    SMOOTH_SIGMOID,
    SMOOTH_SMOOTHSTEP,
    SMOOTH_EASEINOUTQUAD
} SmoothMode;

// 几种平滑函数
float MathTool_Sigmoid(float x, float k);
float MathTool_Smoothstep(float edge0, float edge1, float x);
float MathTool_EaseInOutQuad(float t);
float MathTool_ExponentialSmoothing(float newValue, float oldValue, float alpha);

#endif //STARSECTOR_NANO_MATHTOOL_H
