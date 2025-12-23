#include "Distortion.h"
#include <windows.h>
#include <string.h>
#include <math.h>
#include <minwindef.h>
#include <ntdef.h>
#include <pathcch.h>

// 内部函数：计算下一帧波幅
static void nextFrame(RippleEffect_t* effect) {
    int width = effect->width;
    int height = effect->height;
    short* buf = effect->buf;
    short* buf2 = effect->buf2;

    for (int i = width; i < height * (width - 1); i++) {
        buf2[i] = ((buf[i - width] + buf[i + width] + buf[i - 1] + buf[i + 1]) >> 1) - buf2[i];
        buf2[i] -= buf2[i] >> 5;
    }

    short* temp = buf;
    effect->buf = buf2;
    effect->buf2 = temp;
}

// 内部函数：处理波幅影响后的位图
static void renderRipple(RippleEffect_t* effect) {
    int width = effect->width;
    int height = effect->height;
    DWORD* img_ptr1 = effect->img_ptr1;
    DWORD* img_ptr2 = effect->img_ptr2;
    short* buf = effect->buf;

    int i = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            short data = 1024 - buf[i];

            int a = ((x - width / 2) * data / 1024) + width / 2;
            int b = ((y - height / 2) * data / 1024) + height / 2;

            if (a >= width) a = width - 1;
            if (a < 0) a = 0;
            if (b >= height) b = height - 1;
            if (b < 0) b = 0;

            img_ptr2[i] = img_ptr1[a + (b * width)];
            i++;
        }
    }
}

// 初始化水波效果
bool RippleEffect_Init(RippleEffect_t* effect, int width, int height, const char* imagePath) {
    effect->width = width;
    effect->height = height;

    // 初始化图片
    //initgraph(width, height);
    loadimage(&effect->src_img, imagePath);
    effect->dest_img.Resize(width, height);

    // 获取内存指针
    effect->img_ptr1 = GetImageBuffer(&effect->src_img);
    effect->img_ptr2 = GetImageBuffer(&effect->dest_img);

    // 分配波幅数组
    effect->buf = (short*)malloc((height * width + width) * sizeof(short));
    effect->buf2 = (short*)malloc((height * width + width) * sizeof(short));
    memset(effect->buf, 0, (height * width + width) * sizeof(short));
    memset(effect->buf2, 0, (height * width + width) * sizeof(short));

    return true;
}

// 初始化水波效果
bool RippleEffect_Init(RippleEffect_t* effect, ResourceManager* rm, const char* name) {
    ResourceManager_GetIMAGE(rm, &effect->src_img, "engine", name);
    int width = effect->src_img.getwidth();
    int height = effect->src_img.getheight();
    effect->width = width;
    effect->height = height;

    // 初始化图片
    effect->dest_img.Resize(width, height);

    // 获取内存指针
    effect->img_ptr1 = GetImageBuffer(&effect->src_img);
    effect->img_ptr2 = GetImageBuffer(&effect->dest_img);

    // 分配波幅数组
    effect->buf = (short*)malloc((height * width + width) * sizeof(short));
    effect->buf2 = (short*)malloc((height * width + width) * sizeof(short));
    memset(effect->buf, 0, (height * width + width) * sizeof(short));
    memset(effect->buf2, 0, (height * width + width) * sizeof(short));

    return true;
}

// 初始化水波效果
bool RippleEffect_Init(RippleEffect_t* effect, IMAGE* image) {
    effect->src_img = *image;
    int width = effect->src_img.getwidth();
    int height = effect->src_img.getheight();
    effect->width = width;
    effect->height = height;

    // 初始化图片
    effect->dest_img.Resize(width, height);

    // 获取内存指针
    effect->img_ptr1 = GetImageBuffer(&effect->src_img);
    effect->img_ptr2 = GetImageBuffer(&effect->dest_img);

    // 分配波幅数组
    effect->buf = (short*)malloc((height * width + width) * sizeof(short));
    effect->buf2 = (short*)malloc((height * width + width) * sizeof(short));
    memset(effect->buf, 0, (height * width + width) * sizeof(short));
    memset(effect->buf2, 0, (height * width + width) * sizeof(short));

    return true;
}


// 更新波幅数组
void RippleEffect_Update(RippleEffect_t* effect) {
    nextFrame(effect);
}

// 渲染扭曲效果到目标图像
void RippleEffect_Render(RippleEffect_t* effect, IMAGE* destImage) {
    renderRipple(effect);
    *destImage = effect->dest_img;
}

// 模拟鼠标点击或移动产生的波纹(请注意，这里的x和y是相对图片的坐标系，不是世界坐标系！)

void RippleEffect_Disturb(RippleEffect_t* effect, int x, int y, int stoneSize, int stoneWeight) {
    int width = effect->width;
    int height = effect->height;
    short* buf = effect->buf;

    if ((x >= width - stoneSize) || (x < stoneSize) ||
        (y >= height - stoneSize) || (y < stoneSize)) {
        return;
    }

    for (int posx = x - stoneSize; posx < x + stoneSize; posx++) {
        for (int posy = y - stoneSize; posy < y + stoneSize; posy++) {
            if ((posx - x) * (posx - x) + (posy - y) * (posy - y) < stoneSize * stoneSize) {
                buf[width * posy + posx] += stoneWeight;
            }
        }
    }
}

// 支持世界坐标系的扩展函数
void RippleEffect_Disturb_World(RippleEffect_t* effect, int worldX, int worldY, int stoneSize, int stoneWeight, int imageX, int imageY) {
    // 获取图像尺寸
    int width = effect->width;
    int height = effect->height;

    // 将世界坐标转换为局部坐标
    // 注意：imageX 和 imageY 是图片的中心点
    int localX = worldX - (imageX - width / 2);  // 计算相对于图片左上角的 X 偏移
    int localY = worldY - (imageY - height / 2); // 计算相对于图片左上角的 Y 偏移

    // 检查局部坐标是否在图像范围内
    if (localX < 0 || localX >= width || localY < 0 || localY >= height) {
        return; // 如果不在范围内，直接返回
    }

    // 调整涟漪生成的中心点位置
    // 确保涟漪效果出现在鼠标点击的实际位置
    RippleEffect_Disturb(effect, localX, localY, stoneSize, stoneWeight);
}

// 初始化滚动扭曲效果
bool DistortionEffect_Init(DistortionEffect_t* effect, int width, int height, const char* imagePath) {
    effect->width = width;
    effect->height = height;

    // 加载原始图片
    loadimage(&effect->src_img, imagePath);
    effect->dest_img.Resize(width, height);

    // 获取内存指针
    effect->img_ptr1 = GetImageBuffer(&effect->src_img);
    effect->img_ptr2 = GetImageBuffer(&effect->dest_img);

    // 初始化动态参数
    effect->time = 0.0f;
    effect->amplitude = 12.0f;    // 默认振幅8像素
    effect->frequency = 0.10f;   // 波形频率
    effect->speed = -0.3f;        // 滚动速度

    return true;
}

bool DistortionEffect_Init(DistortionEffect_t* effect, ResourceManager* rm, const char* name) {
    ResourceManager_GetIMAGE(rm, &effect->src_img, "engine", name);
    int width = effect->src_img.getwidth();
    int height = effect->src_img.getheight();
    effect->width = width;
    effect->height = height;

    effect->dest_img.Resize(width, height);

    // 获取内存指针
    effect->img_ptr1 = GetImageBuffer(&effect->src_img);
    effect->img_ptr2 = GetImageBuffer(&effect->dest_img);

    // 初始化动态参数
    effect->time = 0.0f;
    effect->amplitude = 12.0f;    // 默认振幅8像素
    effect->frequency = 0.10f;   // 波形频率
    effect->speed = -0.3f;        // 滚动速度

    return true;
}

// 更新扭曲参数（每帧调用）
void DistortionEffect_Update(DistortionEffect_t* effect) {
    effect->time += 1.0f; // 时间累计
}

// 渲染S型扭曲效果
void DistortionEffect_Render_S(DistortionEffect_t* effect, IMAGE* destImage) {
    int width = effect->width;
    int height = effect->height;
    DWORD* src = effect->img_ptr1;
    DWORD* dst = effect->img_ptr2;

    const int midX = width / 2;       // 水平中线
    const float phaseBase = effect->time * effect->speed; // 基础相位

        int pixelIndex = 0;
    for (int y = 0; y < height; ++y) {
        // 计算当前行的动态相位（带垂直滚动效果）
        const float verticalPhase = phaseBase + y * effect->frequency;

        for (int x = 0; x < width; ++x) {
            // 计算波形偏移
            float wave = sin(verticalPhase);
            float offset;

            // 左右半区不同方向
            if (x < midX) {
                // 左半边：振幅从中间到左侧逐渐增强
                offset = wave * effect->amplitude * (midX - x) / (float)midX;
            } else {
                // 右半边：振幅从中间到右侧逐渐增强，且方向相反
                offset = -wave * effect->amplitude * (x - midX) / (float)midX;
            }

            // 计算源坐标
            int srcX = x + (int)offset;
            int srcY = y;

            // 边界保护
            srcX = (srcX < 0) ? 0 : (srcX >= width) ? width - 1 : srcX;
            srcY = (srcY < 0) ? 0 : (srcY >= height) ? height - 1 : srcY;

            // 写入目标像素
            dst[pixelIndex] = src[srcY * width + srcX];
            ++pixelIndex;
        }
    }

    // 将结果复制到目标图像
    *destImage = effect->dest_img;
}


// 双线性插值缩放
void Distortion_ZoomImage(IMAGE* result, IMAGE* origin, double ZoomRate, bool HighQuality, double ZoomRate2)
{
    // 参数初始化
    if (ZoomRate2 == 0) ZoomRate2 = ZoomRate;
    int srcW = origin->getwidth();
    int srcH = origin->getheight();
    int dstW = static_cast<int>(srcW * ZoomRate);
    int dstH = static_cast<int>(srcH * ZoomRate2);
    result->Resize(dstW, dstH);

    DWORD* dstBuf = GetImageBuffer(result);
    DWORD* srcBuf = GetImageBuffer(origin);

    if (HighQuality)
    {
        for (int y = 0; y < dstH; ++y)
        {
            for (int x = 0; x < dstW; ++x)
            {
                // 计算原始坐标（带边界保护）
                double srcX = (x + 0.5) / ZoomRate - 0.5;
                double srcY = (y + 0.5) / ZoomRate2 - 0.5;

                // 确保坐标在合法范围内
                int x1 = static_cast<int>(srcX);
                int y1 = static_cast<int>(srcY);
                x1 = max(0, min(x1, srcW - 2));  // 防止越界
                y1 = max(0, min(y1, srcH - 2));

                // 获取四个采样点
                DWORD p1 = srcBuf[x1 + y1 * srcW];
                DWORD p2 = srcBuf[(x1+1) + y1 * srcW];
                DWORD p3 = srcBuf[x1 + (y1+1) * srcW];
                DWORD p4 = srcBuf[(x1+1) + (y1+1) * srcW];

                // 计算各通道平均值
                BYTE a = static_cast<BYTE>((
                                                   GetAValue(p1) + GetAValue(p2) +
                                                   GetAValue(p3) + GetAValue(p4)) / 4);

                BYTE r = static_cast<BYTE>((
                                                   GetRValue(p1) + GetRValue(p2) +
                                                   GetRValue(p3) + GetRValue(p4)) / 4);

                BYTE g = static_cast<BYTE>((
                                                   GetGValue(p1) + GetGValue(p2) +
                                                   GetGValue(p3) + GetGValue(p4)) / 4);

                BYTE b = static_cast<BYTE>((
                                                   GetBValue(p1) + GetBValue(p2) +
                                                   GetBValue(p3) + GetBValue(p4)) / 4);

                // 写入目标缓冲区
                dstBuf[x + y * dstW] = ARGB(a, r, g, b);
            }
        }
    }
    else
    {
        // 低质量缩放（保持原始代码）
        for (int y = 0; y < dstH; ++y) {
            for (int x = 0; x < dstW; ++x) {
                int srcX = static_cast<int>(x / ZoomRate);
                int srcY = static_cast<int>(y / ZoomRate2);
                srcX = max(0, min(srcX, srcW - 1));
                srcY = max(0, min(srcY, srcH - 1));
                dstBuf[x + y * dstW] = srcBuf[srcX + srcY * srcW];
            }
        }
    }
}