#include "ThreadPool.h"

// 定义全局线程池变量
ExplosionThreadPool thread_pool;

void* explosion_worker(void* arg) {
    while (1) {
        ExplosionTask task;
        int got_task = 0;

        pthread_mutex_lock(&thread_pool.task_mutex);

        while (thread_pool.task_count == 0 && !thread_pool.shutdown) {
            pthread_cond_wait(&thread_pool.task_cond, &thread_pool.task_mutex);
        }

        if (thread_pool.shutdown) {
            pthread_mutex_unlock(&thread_pool.task_mutex);
            pthread_exit(NULL);
        }

        if (thread_pool.task_count > 0) {
            task = thread_pool.tasks[--thread_pool.task_count];
            got_task = 1;
        }

        pthread_mutex_unlock(&thread_pool.task_mutex);

        if (got_task) {
            pthread_mutex_lock(&task.explosion->mutex);
            bool is_active = ExplosionAPI_UpdateExplosion(task.explosion, task.deltaTime);
            pthread_mutex_unlock(&task.explosion->mutex);

            if (!is_active) {
                task.explosion->is_active = 0;
            }
        }
    }
    return NULL;
}

void ExplosionThreadPool_Init(int max_threads) {
    thread_pool.task_count = 0;
    thread_pool.shutdown = 0;

    pthread_mutex_init(&thread_pool.task_mutex, NULL);
    pthread_cond_init(&thread_pool.task_cond, NULL);

    int actual_threads = max_threads < MAX_THREADS ? max_threads : MAX_THREADS;
    for (int i = 0; i < actual_threads; i++) {
        pthread_create(&thread_pool.threads[i], NULL, explosion_worker, NULL);
    }
}

void ExplosionThreadPool_Shutdown() {
    pthread_mutex_lock(&thread_pool.task_mutex);
    thread_pool.shutdown = 1;
    pthread_mutex_unlock(&thread_pool.task_mutex);

    pthread_cond_broadcast(&thread_pool.task_cond);

    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread_pool.threads[i]) {
            pthread_join(thread_pool.threads[i], NULL);
        }
    }

    pthread_mutex_destroy(&thread_pool.task_mutex);
    pthread_cond_destroy(&thread_pool.task_cond);
}

void ExplosionPool_AddTask(ExplosionAPI* explosion, float deltaTime) {
    pthread_mutex_lock(&thread_pool.task_mutex);

    if (thread_pool.task_count < 1000) {
        thread_pool.tasks[thread_pool.task_count].explosion = explosion;
        thread_pool.tasks[thread_pool.task_count].deltaTime = deltaTime;
        thread_pool.task_count++;

        pthread_cond_signal(&thread_pool.task_cond);
    }

    pthread_mutex_unlock(&thread_pool.task_mutex);
}