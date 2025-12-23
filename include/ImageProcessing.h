#ifndef STARSECTOR_NANO_IMAGEPROCESSING_H
#define STARSECTOR_NANO_IMAGEPROCESSING_H

#include <math.h>
#include "easyx.h"
#include "Distortion.h"

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

/* 自定义通道值提取宏 */
#define GET_B(color) ((color) & 0xFF)         // 蓝通道
#define GET_G(color) ((color >> 8) & 0xFF)    // 绿通道
#define GET_R(color) ((color >> 16) & 0xFF)   // 红通道
#define GET_A(color) ((color >> 24) & 0xFF)   // Alpha通道

/* 合成BGRA颜色值宏 */
#define MAKE_BGRA(a, r, g, b) \
    ((DWORD)((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

void ImageProcessing_BlendImages(IMAGE* result, IMAGE* light_image, IMAGE* original_image);
// 透明度调整
void ImageProcessing_AdjustImageTransparency(IMAGE* result, IMAGE* image, float transparencyFactor);

// 高斯模糊
void ImageProcessing_GaussianBlurInPlace(
        IMAGE* src,
        int radius,
        float sigma,
        bool blurAlpha = false  // 默认不模糊Alpha通道
);
// 平滑过渡函数,用于控制模糊强度变化
static inline float smoothstep(float edge0, float edge1, float x) {
    x = fminf(fmaxf((x - edge0) / (edge1 - edge0), 0.0f), 1.0f);
    return x * x * (3.0f - 2.0f * x);
}
// Kawase Blur - 快速近似模糊算法
void ImageProcessing_FastKawaseBlur(IMAGE* src, int radius, bool blurAlpha);
// 带混合权重的图像混合
void ImageProcessing_BlendImagesWithWeight(
        IMAGE* result,
        IMAGE* src1,
        IMAGE* src2,
        float weight);
#endif //STARSECTOR_NANO_IMAGEPROCESSING_H
