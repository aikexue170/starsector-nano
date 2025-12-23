// Timer.h
#ifndef TIMER_H
#define TIMER_H

#include <windows.h>

typedef struct {
    LARGE_INTEGER start_time;
    LARGE_INTEGER pause_time;
    LARGE_INTEGER frequency;
    BOOL has_perf_counter;
    double time_scale;
    BOOL paused;
} Timer;

void Timer_Init(Timer* timer);
void Timer_Start(Timer* timer);
void Timer_Pause(Timer* timer);
void Timer_Resume(Timer* timer);
double Timer_GetElapsedSeconds(const Timer* timer);
double Timer_GetElapsedMilliseconds(const Timer* timer);
void Timer_SetTimeScale(Timer* timer, double scale);
double Timer_GetFPS(void);
double DeltaTime_Get(void); // 获取上一帧到当前帧的时间差（秒）
void Timer_Destroy(Timer* timer);

#endif
