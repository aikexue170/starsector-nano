#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include "ExplosionAPI.h"

#define MAX_THREADS 4

typedef struct {
    ExplosionAPI* explosion;
    float deltaTime;
} ExplosionTask;

typedef struct {
    ExplosionTask tasks[1000];
    int task_count;
    pthread_mutex_t task_mutex;
    pthread_cond_t task_cond;
    pthread_t threads[MAX_THREADS];
    int shutdown;
} ExplosionThreadPool;

// 声明全局线程池变量
extern ExplosionThreadPool thread_pool;

// 声明线程池函数
void* explosion_worker(void* arg);
void ExplosionThreadPool_Init(int max_threads);
void ExplosionThreadPool_Shutdown();
void ExplosionPool_AddTask(ExplosionAPI* explosion, float deltaTime);

#endif // THREAD_POOL_H
