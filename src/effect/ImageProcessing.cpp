#include "ImageProcessing.h"


void ImageProcessing_BlendImages(IMAGE* result, IMAGE* light_image, IMAGE* original_image) {
    const int width = light_image->getwidth();
    const int height = light_image->getheight();

    /* 验证图像尺寸 */
    if (original_image->getwidth() != width || original_image->getheight() != height) {
        return;
    }

    /* 调整结果图像尺寸 */
    result->Resize(width, height);

    /* 获取像素缓冲区 */
    DWORD *srcBuf = GetImageBuffer(light_image);
    DWORD *lightBuf = GetImageBuffer(original_image);
    DWORD *dstBuf = GetImageBuffer(result);

    /* 遍历所有像素 */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int index = x + y * width;

            /* 分解原图像素 */
            const DWORD srcColor = srcBuf[index];
            const BYTE srcB = GET_B(srcColor);
            const BYTE srcG = GET_G(srcColor);
            const BYTE srcR = GET_R(srcColor);
            const BYTE srcA = GET_A(srcColor);

            /* 分解光效图像素 */
            const DWORD lightColor = lightBuf[index];
            const BYTE lightB = GET_B(lightColor);
            const BYTE lightG = GET_G(lightColor);
            const BYTE lightR = GET_R(lightColor);
            const BYTE lightA = GET_A(lightColor);

            /* 预乘Alpha到光效颜色通道 */
            const int scaledLightR = (lightR * lightA) / 255;
            const int scaledLightG = (lightG * lightA) / 255;
            const int scaledLightB = (lightB * lightA) / 255;

            /* 计算混合Alpha值 */
            float srcAlphaFactor = srcA / 255.0f;
            float lightAlphaFactor = lightA / 255.0f;
            float finalAlpha = srcAlphaFactor + lightAlphaFactor * (1 - srcAlphaFactor);

            /* 根据Alpha因子调整颜色混合 */
            const int blendedR = (BYTE)(srcR * srcAlphaFactor + scaledLightR * lightAlphaFactor * (1 - srcAlphaFactor));
            const int blendedG = (BYTE)(srcG * srcAlphaFactor + scaledLightG * lightAlphaFactor * (1 - srcAlphaFactor));
            const int blendedB = (BYTE)(srcB * srcAlphaFactor + scaledLightB * lightAlphaFactor * (1 - srcAlphaFactor));

            /* 通道值限幅 */
            const BYTE finalR = (BYTE) MIN(MAX(blendedR, 0), 255);
            const BYTE finalG = (BYTE) MIN(MAX(blendedG, 0), 255);
            const BYTE finalB = (BYTE) MIN(MAX(blendedB, 0), 255);
            const BYTE finalA = (BYTE)(finalAlpha * 255);

            /* 合成最终像素 */
            dstBuf[index] = MAKE_BGRA(finalA, finalR, finalG, finalB);
        }
    }
}


// 透明度调整
void ImageProcessing_AdjustImageTransparency(IMAGE* result, IMAGE* image, float transparencyFactor) {
    if (transparencyFactor < 0 || transparencyFactor > 1) {
        return;
    }

    const int width = image->getwidth();
    const int height = image->getheight();

    // 确保目标图像大小与原图一致
    result->Resize(width, height);

    DWORD *imgBuf = GetImageBuffer(image);
    DWORD *resBuf = GetImageBuffer(result);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int index = x + y * width;

            const DWORD color = imgBuf[index];
            const BYTE B = GET_B(color);
            const BYTE G = GET_G(color);
            const BYTE R = GET_R(color);
            const BYTE A = GET_A(color);

            // 计算新的透明度值
            const BYTE newA = static_cast<BYTE>(A * transparencyFactor);

            // 将新颜色写入目标缓冲区
            resBuf[index] = MAKE_BGRA( newA, R, G, B);
        }
    }
}

// 高斯模糊
void ImageProcessing_GaussianBlurInPlace(IMAGE* src, int radius, float sigma, bool blurAlpha) {
    const int width = src->getwidth();
    const int height = src->getheight();
    DWORD* srcBuf = GetImageBuffer(src);

    // 计算高斯核
    int kernelSize = radius * 2 + 1;
    float* kernel = new float[kernelSize];
    float sum = 0.0f;

    // 生成一维高斯核（分离式高斯模糊优化）
    for (int i = -radius; i <= radius; i++) {
        float value = exp(-(i * i) / (2 * sigma * sigma));
        kernel[i + radius] = value;
        sum += value;
    }

    // 归一化
    for (int i = 0; i < kernelSize; i++) {
        kernel[i] /= sum;
    }

    // 临时缓冲区
    DWORD* tempBuf = new DWORD[width * height];

    // 水平模糊
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float r = 0, g = 0, b = 0, a = 0;

            for (int kx = -radius; kx <= radius; kx++) {
                int px = min(max(x + kx, 0), width - 1);
                DWORD pixel = srcBuf[px + y * width];

                float weight = kernel[kx + radius];
                r += GET_R(pixel) * weight;
                g += GET_G(pixel) * weight;
                b += GET_B(pixel) * weight;
                if (blurAlpha) {
                    a += GET_A(pixel) * weight;
                }
            }

            // 如果不需要模糊Alpha，保留原Alpha值
            BYTE finalA = blurAlpha ? (BYTE)min(max(a, 0.0f), 255.0f) : GET_A(srcBuf[x + y * width]);

            tempBuf[x + y * width] = MAKE_BGRA(
                    finalA,
                    (BYTE)min(max(r, 0.0f), 255.0f),
                    (BYTE)min(max(g, 0.0f), 255.0f),
                    (BYTE)min(max(b, 0.0f), 255.0f)
            );
        }
    }

    // 垂直模糊（直接写回原图）
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float r = 0, g = 0, b = 0, a = 0;

            for (int ky = -radius; ky <= radius; ky++) {
                int py = min(max(y + ky, 0), height - 1);
                DWORD pixel = tempBuf[x + py * width];

                float weight = kernel[ky + radius];
                r += GET_R(pixel) * weight;
                g += GET_G(pixel) * weight;
                b += GET_B(pixel) * weight;
                if (blurAlpha) {
                    a += GET_A(pixel) * weight;
                }
            }

            BYTE finalA = blurAlpha ? (BYTE)min(max(a, 0.0f), 255.0f) : GET_A(srcBuf[x + y * width]);

            srcBuf[x + y * width] = MAKE_BGRA(
                    finalA,
                    (BYTE)min(max(r, 0.0f), 255.0f),
                    (BYTE)min(max(g, 0.0f), 255.0f),
                    (BYTE)min(max(b, 0.0f), 255.0f)
            );
        }
    }

    delete[] kernel;
    delete[] tempBuf;
}

void ImageProcessing_BlendImagesWithWeight(IMAGE* result, IMAGE* src1, IMAGE* src2, float weight) {
    const int width = src1->getwidth();
    const int height = src1->getheight();

    if (src2->getwidth() != width || src2->getheight() != height) {
        return;
    }

    result->Resize(width, height);

    DWORD *src1Buf = GetImageBuffer(src1);
    DWORD *src2Buf = GetImageBuffer(src2);
    DWORD *dstBuf = GetImageBuffer(result);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int index = x + y * width;

            DWORD color1 = src1Buf[index];
            DWORD color2 = src2Buf[index];

            BYTE r1 = GET_R(color1), r2 = GET_R(color2);
            BYTE g1 = GET_G(color1), g2 = GET_G(color2);
            BYTE b1 = GET_B(color1), b2 = GET_B(color2);
            BYTE a1 = GET_A(color1), a2 = GET_A(color2);

            BYTE finalR = (BYTE)(r1 * (1-weight) + r2 * weight);
            BYTE finalG = (BYTE)(g1 * (1-weight) + g2 * weight);
            BYTE finalB = (BYTE)(b1 * (1-weight) + b2 * weight);
            BYTE finalA = (BYTE)(a1 * (1-weight) + a2 * weight);

            dstBuf[index] = MAKE_BGRA(finalA, finalR, finalG, finalB);
        }
    }
}

// Kawase Blur - 快速近似模糊算法
void ImageProcessing_FastKawaseBlur(IMAGE* src, int radius, bool blurAlpha) {
    const int width = src->getwidth();
    const int height = src->getheight();
    DWORD* srcBuf = GetImageBuffer(src);

    // 创建临时缓冲区
    DWORD* tempBuf = new DWORD[width * height];

    // 模糊迭代次数 (可根据radius调整)
    int iterations = min(3, max(1, radius / 2));

    for (int i = 0; i < iterations; i++) {
        int offset = 1 + i;  // 逐渐增大的采样偏移

        // 水平+垂直混合模糊
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // 采样周围4个像素(对角线方向)
                int x1 = max(0, x - offset);
                int x2 = min(width-1, x + offset);
                int y1 = max(0, y - offset);
                int y2 = min(height-1, y + offset);

                DWORD p1 = srcBuf[x1 + y1 * width];
                DWORD p2 = srcBuf[x2 + y1 * width];
                DWORD p3 = srcBuf[x1 + y2 * width];
                DWORD p4 = srcBuf[x2 + y2 * width];

                // 平均值混合
                BYTE r = (GET_R(p1) + GET_R(p2) + GET_R(p3) + GET_R(p4)) / 4;
                BYTE g = (GET_G(p1) + GET_G(p2) + GET_G(p3) + GET_G(p4)) / 4;
                BYTE b = (GET_B(p1) + GET_B(p2) + GET_B(p3) + GET_B(p4)) / 4;
                BYTE a = blurAlpha ?
                         (GET_A(p1) + GET_A(p2) + GET_A(p3) + GET_A(p4)) / 4 :
                         GET_A(srcBuf[x + y * width]);

                tempBuf[x + y * width] = MAKE_BGRA(a, r, g, b);
            }
        }

        // 交换缓冲区(准备下一次迭代)
        memcpy(srcBuf, tempBuf, width * height * sizeof(DWORD));
    }

    delete[] tempBuf;
}