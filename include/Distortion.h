#ifndef DISTORTION_H
#define DISTORTION_H

#include <graphics.h>
#include "Manager.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define GetAValue(argb) ((argb >> 24) & 0xFF)

// 原错误定义（RGBA顺序）：
// #define ARGB(a, r, g, b) ((a<<24)|(r<<16)|(g<<8)|b)

// 修正为BGRA顺序（匹配EasyX）：
#define ARGB(a, r, g, b) ((DWORD(a)<<24) | (DWORD(b)<<16) | (DWORD(g)<<8) | DWORD(r))
//                        Alpha通道      Blue通道          Green通道      Red通道
/**
* @CreateTime 2025/3/4
* @Author 刘
* @brief 关于RippleEffect库的使用，这里有必要做些说明
 * 首先使用RippleEffect_t effect初始化effect
 * 其次，RippleEffect_Init做初始化
 * 定义好destImage
 * 在主循环中使用RippleEffect_Update(&effect)和RippleEffect_Render(&effect, &destImage)
 * Render方法会把处理后的图像返回到destImage中，指针操作，无需赋值
 * 需要涟漪时，使用Disturb方法。size可以选小一点，但是weight最好选250往上，上千也可以
 */

// 定义水波效果的结构体
typedef struct {
    IMAGE src_img;       // 原始图片
    IMAGE dest_img;      // 处理后的图片
    DWORD* img_ptr1;     // 原始图片内存指针
    DWORD* img_ptr2;     // 处理后图片内存指针
    short* buf;          // 当前波幅数组
    short* buf2;         // 下一时刻波幅数组
    int width, height;   // 图片尺寸
} RippleEffect_t;

typedef struct {
    IMAGE src_img;
    IMAGE dest_img;
    DWORD* img_ptr1;
    DWORD* img_ptr2;
    int width, height;
    float  time;         // 动画时间累计
    float  amplitude;   // 扭曲幅度（像素）
    float  frequency;   // 波形频率
    float  speed;       // 滚动速度
} DistortionEffect_t;

// 初始化水波效果
bool RippleEffect_Init(RippleEffect_t* effect, int width, int height, const char* imagePath);
bool RippleEffect_Init(RippleEffect_t* effect, ResourceManager* rm, const char* name);// 更新波幅数组
bool RippleEffect_Init(RippleEffect_t* effect, IMAGE* image);
void RippleEffect_Update(RippleEffect_t* effect);

// 渲染扭曲效果到目标图像
void RippleEffect_Render(RippleEffect_t* effect, IMAGE* destImage);

// 产生波纹
void RippleEffect_Disturb(RippleEffect_t* effect, int x, int y, int stoneSize, int stoneWeight);


void RippleEffect_Disturb_World(RippleEffect_t* effect, int worldX, int worldY, int stoneSize, int stoneWeight, int imageX, int imageY);


bool DistortionEffect_Init(DistortionEffect_t* effect, int width, int height, const char* imagePath);
bool DistortionEffect_Init(DistortionEffect_t* effect, ResourceManager* rm, const char* name);
void DistortionEffect_Update(DistortionEffect_t* effect);

// S形扭曲波动
void DistortionEffect_Render_S(DistortionEffect_t* effect, IMAGE* destImage);

void Distortion_ZoomImage(IMAGE* result, IMAGE* origin, double ZoomRate, bool HighQuality=false, double ZoomRate2=0);

DWORD GetImagePixel(IMAGE* img, int x, int y);
#endif // DISTORTION_H