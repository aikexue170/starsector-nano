#ifndef STARSECTOR_NANO_SEQUENCEDEXPLOSION_H
#define STARSECTOR_NANO_SEQUENCEDEXPLOSION_H
#include "ExplosionAPI.h"
#include "Vector2f.h"
#include "ExplosionPool.h"
#include <stdbool.h>

// 爆炸序列配置
typedef struct {
    Vector2f center;       // 爆炸中心点
    float radius;          // 爆炸随机范围半径
    int minExplosions;     // 最小爆炸次数
    int maxExplosions;     // 最大爆炸次数
    float minDelay;        // 两次爆炸间最小延迟（秒）
    float maxDelay;        // 两次爆炸间最大延迟（秒）
    float duration;        // 单次爆炸持续时间
    ExplosionType type;    // 爆炸类型
    bool isLooping;        // 是否循环播放
} ExplosionSequenceConfig;

// 单个爆炸序列实例
typedef struct {
    ExplosionSequenceConfig config;
    float timer;           // 计时器，控制爆炸间隔
    int remainingExplosions; // 剩余爆炸次数
    bool isActive;         // 是否正在播放
} ExplosionSequence;

// 爆炸序列对象池
#define MAX_SEQUENCED_EXPLOSIONS 64  // 最大同时存在的爆炸序列

typedef struct {
    ExplosionSequence sequences[MAX_SEQUENCED_EXPLOSIONS];
    bool inUse[MAX_SEQUENCED_EXPLOSIONS];
} SequencedExplosionPool;

// 初始化对象池
void SequencedExplosionPool_Init(SequencedExplosionPool* pool);

// 从对象池获取一个可用的爆炸序列
ExplosionSequence* SequencedExplosionPool_Acquire(SequencedExplosionPool* pool);

// 释放爆炸序列回对象池
void SequencedExplosionPool_Release(SequencedExplosionPool* pool, ExplosionSequence* seq);

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
);

// 更新爆炸序列（每帧调用）
void ExplosionSequence_Update(
        ExplosionSequence* seq,
        ExplosionPool* explosionPool,
        ResourceManager* rsm,
        RenderManager* rdm,
        float deltaTime
);

// 停止爆炸序列（可提前终止）
void ExplosionSequence_Stop(ExplosionSequence* seq);
#endif //STARSECTOR_NANO_SEQUENCEDEXPLOSION_H
