#include "SequencedExplosion.h"
#include <stdlib.h> // for rand()
#include <string.h> // for memset

// 初始化对象池
void SequencedExplosionPool_Init(SequencedExplosionPool* pool) {
    memset(pool, 0, sizeof(SequencedExplosionPool)); // 全部置零
}

// 从对象池获取一个可用的爆炸序列
ExplosionSequence* SequencedExplosionPool_Acquire(SequencedExplosionPool* pool) {
    for (int i = 0; i < MAX_SEQUENCED_EXPLOSIONS; i++) {
        if (!pool->inUse[i]) {
            pool->inUse[i] = true;
            memset(&pool->sequences[i], 0, sizeof(ExplosionSequence)); // 确保初始化
            return &pool->sequences[i];
        }
    }
    return NULL; // 池已满
}

// 释放爆炸序列回对象池
void SequencedExplosionPool_Release(SequencedExplosionPool* pool, ExplosionSequence* seq) {
    for (int i = 0; i < MAX_SEQUENCED_EXPLOSIONS; i++) {
        if (&pool->sequences[i] == seq) {
            pool->inUse[i] = false;
            return;
        }
    }
}

// 开始播放爆炸序列
void ExplosionSequence_Start(
        ExplosionSequence* seq,
        Vector2f center,
        float radius,
        int minExplosions,
        int maxExplosions,
        float minDelay,
        float maxDelay,
        float duration,
        ExplosionType type,
        bool isLooping
) {
    seq->config.center = center;
    seq->config.radius = radius;
    seq->config.minExplosions = minExplosions;
    seq->config.maxExplosions = maxExplosions;
    seq->config.minDelay = minDelay;
    seq->config.maxDelay = maxDelay;
    seq->config.duration = duration;
    seq->config.type = type;
    seq->config.isLooping = isLooping;

    seq->remainingExplosions = minExplosions + (rand() % (maxExplosions - minExplosions + 1));
    seq->timer = 0.0f;
    seq->isActive = true;
}

// 更新爆炸序列
void ExplosionSequence_Update(
        ExplosionSequence* seq,
        ExplosionPool* explosionPool,
        ResourceManager* rsm,
        RenderManager* rdm,
        float deltaTime
) {
    if (!seq->isActive) return;

    seq->timer -= deltaTime;

    if (seq->timer <= 0.0f && seq->remainingExplosions > 0) {
        // 计算随机位置（圆形范围内）
        float angle = (float)(rand() % 360) * 3.14159265f / 180.0f;
        float distance = (float)(rand() % 100) / 100.0f * seq->config.radius;
        Vector2f offset = { cosf(angle) * distance, sinf(angle) * distance };
        Vector2f explosionPos = {
                seq->config.center.x + offset.x,
                seq->config.center.y + offset.y
        };

        // 生成爆炸
        ExplosionPool_GenerateExplosion(
                explosionPool,
                rsm,
                rdm,
                seq->config.type,
                explosionPos,
                seq->config.duration
        );

        // 设置下一次爆炸的随机延迟
        seq->remainingExplosions--;
        seq->timer = seq->config.minDelay +
                     (rand() % (int)((seq->config.maxDelay - seq->config.minDelay) * 1000)) / 1000.0f;

        // 如果爆炸次数用完但 isLooping=true，则重新开始
        if (seq->remainingExplosions <= 0 && seq->config.isLooping) {
            seq->remainingExplosions = seq->config.minExplosions +
                                       (rand() % (seq->config.maxExplosions - seq->config.minExplosions + 1));
        }
    }

    // 如果爆炸次数用完且不循环，则停止
    if (seq->remainingExplosions <= 0 && !seq->config.isLooping) {
        seq->isActive = false;
    }
}

// 停止爆炸序列
void ExplosionSequence_Stop(ExplosionSequence* seq) {
    seq->isActive = false;
}