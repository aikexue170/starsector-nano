// Timer.c
#include "Timer.h"
#include <math.h>

void Timer_Init(Timer* timer) {
    timer->has_perf_counter = QueryPerformanceFrequency(&timer->frequency);
    timer->time_scale = 1.0;
    timer->paused = FALSE;
    Timer_Start(timer);
}

void Timer_Start(Timer* timer) {
    if (timer->has_perf_counter) {
        QueryPerformanceCounter(&timer->start_time);
    } else {
        timer->start_time.QuadPart = GetTickCount();
    }
    timer->paused = FALSE;
}

void Timer_Pause(Timer* timer) {
    if (!timer->paused) {
        if (timer->has_perf_counter) {
            QueryPerformanceCounter(&timer->pause_time);
        } else {
            timer->pause_time.QuadPart = GetTickCount();
        }
        timer->paused = TRUE;
    }
}

void Timer_Resume(Timer* timer) {
    if (timer->paused) {
        LARGE_INTEGER now;
        if (timer->has_perf_counter) {
            QueryPerformanceCounter(&now);
            timer->start_time.QuadPart += (now.QuadPart - timer->pause_time.QuadPart);
        } else {
            now.QuadPart = GetTickCount();
            timer->start_time.QuadPart += (now.QuadPart - timer->pause_time.QuadPart);
        }
        timer->paused = FALSE;
    }
}

double Timer_GetElapsedSeconds(const Timer* timer) {
    LARGE_INTEGER now;
    if (timer->has_perf_counter) {
        QueryPerformanceCounter(&now);
        return (double)(now.QuadPart - timer->start_time.QuadPart) /
               (double)timer->frequency.QuadPart * timer->time_scale;
    }
    return (GetTickCount() - timer->start_time.QuadPart) * 1e-3 * timer->time_scale;
}

double Timer_GetElapsedMilliseconds(const Timer* timer) {
    return Timer_GetElapsedSeconds(timer) * 1000.0;
}

void Timer_SetTimeScale(Timer* timer, double scale) {
    if (scale > 0) timer->time_scale = scale;
}

double Timer_GetFPS(void) {
    static LARGE_INTEGER freq = {0};
    static LARGE_INTEGER last = {0};
    static double fps = 0.0;

    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&last);
        return 0.0;
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    const double delta = (double)(now.QuadPart - last.QuadPart) / freq.QuadPart;

    if (delta > 1e-5) { // 最小时间间隔保护
        fps = 0.9 * fps + 0.1 * (1.0 / delta);
    }

    last = now;
    return fps;
}

double DeltaTime_Get(void) {
    static LARGE_INTEGER prev_time = {0};
    static LARGE_INTEGER freq = {0};
    static BOOL use_perf_counter = FALSE;

    // 首次调用初始化
    if (freq.QuadPart == 0) {
        use_perf_counter = QueryPerformanceFrequency(&freq);
        if (!use_perf_counter) freq.QuadPart = 1000; // 降级到毫秒精度
        QueryPerformanceCounter(&prev_time);
    }

    // 获取当前时间
    LARGE_INTEGER now;
    if (use_perf_counter) {
        QueryPerformanceCounter(&now);
    } else {
        now.QuadPart = GetTickCount();
    }

    // 计算差值
    const LONGLONG delta = now.QuadPart - prev_time.QuadPart;
    prev_time = now; // 保存当前时间供下次使用

    // 转换为秒
    return (use_perf_counter) ?
           (double)delta / (double)freq.QuadPart :
           (double)delta * 1e-3;
}

void Timer_Destroy(Timer* timer) {
    timer->has_perf_counter = FALSE;
    timer->time_scale = 1.0;
    timer->paused = FALSE;
    timer->start_time.QuadPart = 0;
}
