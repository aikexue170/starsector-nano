#include <easyx.h>
#include <ctime>
#include "Manager.h"
#include <windows.h>
#include <iostream>
#include "Entity.h"
#include "Timer.h"
#include "Background.h"
#include "Camera.h"
#include "ShipPool.h"
#include "Control.h"
#include "BulletPool.h"
#include "ExplosionPool.h"
#include "GlobalCombatData.h"
#include "CustomFSM.h"
#include "TacticalMap.h"
#include "SequencedExplosion.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// 帧时间
double deltaTime = 0;
// 退出标志
bool game_exit = false;
// ID初始定义
bool showCollision = false;

Vector2f position;

int main() {

    // 初始化随机数种子
    srand((unsigned int)time(NULL));

    // 四线程
    ExplosionThreadPool_Init(4);

    // 初始化图形窗口
    initgraph(SCREEN_WIDTH, SCREEN_HEIGHT);
    // 不建议修改宽高，因为有大量代码直接使用1280*720参数，没弄全局的引擎

    // 初始化管理器
    ResourceManager rsm;
    ResourceManager_Init(&rsm);
    RenderManager rdm;
    RenderManager_Init(&rdm);
    Timer timer;
    Timer_Init(&timer);
    Timer_Start(&timer);
    SequencedExplosionPool explosionSeqPool;
    SequencedExplosionPool_Init(&explosionSeqPool);
    Camera camera;
    ExMessage msg;
    ShipPool* ship_pool = ShipPool_Init();
    BulletPool* bullet_pool = BulletPool_Init();
    ExplosionPool* explosion_pool = ExplosionPool_Init();
    CombatDataPool* combat_data_pool = CombatDataPool_Init();

    FSMContext aiCtx;
    FSM_Init(&aiCtx);

    TacticalMap_Init();

    // JSON统一注册
    ResourceManager_RegisterAllFromJSON(&rsm);

    // 注册区
    // 傻瓜式注册，两行代码就能添加一艘舰船
    ShipAPI* player = ShipPool_GetShip(ship_pool, &rsm, &rdm, "PRI_TransmissionGate", (Vector2f){0, 1200}, 0, (Vector2f){0, -100}, true);
    ShipAPI* our1 = ShipPool_GetShip(ship_pool, &rsm, &rdm, "PRI_TransmissionGate", (Vector2f){-300, 1500}, 0, (Vector2f){0, -100});//一个友军
    ShipAPI* enemy = ShipPool_GetShip(ship_pool, &rsm, &rdm, "RUI_RingProbe", (Vector2f){300, -1500}, 180, (Vector2f){0, 100});
    Background* background = Background_Init(&rsm, &rdm, "background1");

    ShipAPI_AddTeam(player, 1);
    ShipAPI_AddTeam(our1, 1);
    ShipAPI_AddTeam(enemy, 2);
   // ShipAPI_AddTeam(their1, 2);

    //主循环
    while(!game_exit){
        //获取帧时间
        deltaTime = DeltaTime_Get();

        //获取玩家位置
        position = player->entity_data.position;

        // 更新摄像机位置
        Camera_HandleMouseMessage(&camera, &msg, deltaTime);
        RenderManager_UpdateCameraScreenPosition(&camera);
        // 对象池更新区
        ShipPool_Update(&rsm, &rdm, &explosionSeqPool, ship_pool, &camera, deltaTime);
        BulletPool_Update(bullet_pool, &rdm, deltaTime);
        ExplosionPool_UpdateMT(explosion_pool, deltaTime);
        CombatDataPool_Update(combat_data_pool, ship_pool);

        for (int i = 0; i < MAX_SEQUENCED_EXPLOSIONS; i++) {
            if (explosionSeqPool.inUse[i] && explosionSeqPool.sequences[i].isActive) {
                ExplosionSequence_Update(
                        &explosionSeqPool.sequences[i],
                        explosion_pool,
                        &rsm,
                        &rdm,
                        deltaTime
                );

                // 如果序列播放完毕，释放回对象池
                if (!explosionSeqPool.sequences[i].isActive) {
                    SequencedExplosionPool_Release(&explosionSeqPool, &explosionSeqPool.sequences[i]);
                }
            }
        }

        // 设置FSM资源
        FSMResources fsmRes = {
                .expPool = explosion_pool,
                .bulletPool = bullet_pool,
                .renderMgr = &rdm,
                .resMgr = &rsm,
                .deltaTime = deltaTime
        };

        // 更新所有AI飞船
        for (int i = 0; i < MAX_SHIPS; i++) {
            if (ship_pool->inUse[i] && !ship_pool->ships[i]->isPlayer) {
                FSM_Update(ship_pool->ships[i], &aiCtx, combat_data_pool, &fsmRes);
            }
        }

        // 键盘控制检测
        Control_Handler(CONTROL_WASD, player, deltaTime);

        //鼠标左键按下检测控制玩家开火：
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000){
            ShipAPI_Fire(player, explosion_pool, bullet_pool, &rdm, &rsm);
        }

        // 船池实体碰撞检测
        ShipPool_CheckCollisions(ship_pool, &rdm, deltaTime);
        //BulletPool_CheckCollisions(bullet_pool, &rdm, deltaTime);
        Pool_CheckMutualCollisions(bullet_pool, ship_pool, explosion_pool, &rsm, &rdm, deltaTime);

        // 渲染器更新
        ShipPool_Render(ship_pool, &rdm);
        BulletPool_Render(bullet_pool, &rdm);
        ExplosionPool_Render(explosion_pool, &rdm);
        RenderManager_MouseUpdate(&rdm, camera.position);
        RenderManager_PlayerUpdate(&rdm, position);
        Camera_UpdatePlayerPosition(&camera, position);

        TacticalMap_Update();

        // 渲染
        BeginBatchDraw();
        cleardevice();
        RenderManager_Render(&rdm);
        TacticalMap_Render(&rsm, &camera, player, combat_data_pool);
        EndBatchDraw();
    }
    // 关闭线程池
    ExplosionThreadPool_Shutdown();
}

