#include "MathTool.h"

// 辅助函数，限制值在给定范围内
static float MathTool_Clamp(float value, float min, float max) {
    return fmaxf(fminf(value, max), min);
}
float MathTool_Sigmoid(float x, float k) {
    return 1.0f / (1.0f + expf(-k * x));
}
float MathTool_Smoothstep(float edge0, float edge1, float x) {
    // Normalize x to [0, 1]
    float t = MathTool_Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
float MathTool_EaseInOutQuad(float t) {
    if (t < 0.5f) {
        return 2.0f * t * t;
    } else {
        return -2.0f * t * t + 4.0f * t - 1.0f;
    }
}
float MathTool_ExponentialSmoothing(float newValue, float oldValue, float alpha) {
    return alpha * newValue + (1.0f - alpha) * oldValue;
}
