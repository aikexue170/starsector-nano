#include "Control.h"


void Control_Handler(ControlType controlType, ShipAPI* targetObject, float deltaTime) {
    switch (controlType) {
        case CONTROL_WASD:
            if (GetAsyncKeyState('W') & 0x8000) {
                ShipAPI_Forward(targetObject, deltaTime);
            } else {
                ShipAPI_EngineDown(targetObject);
            }
            if (GetAsyncKeyState('A') & 0x8000) {
                ShipAPI_TurnLeft(targetObject, deltaTime);
            }
            if (GetAsyncKeyState('S') & 0x8000) {
                ShipAPI_Backward(targetObject, deltaTime);
            }
            if (GetAsyncKeyState('D') & 0x8000) {
                ShipAPI_TurnRight(targetObject, deltaTime);
            }
            break;
        case CONTROL_IJKL:
            if (GetAsyncKeyState('I') & 0x8000) {
                ShipAPI_Forward(targetObject, deltaTime);
            } else {
                ShipAPI_EngineDown(targetObject);
            }
            if (GetAsyncKeyState('J') & 0x8000) {
                ShipAPI_TurnLeft(targetObject, deltaTime);
            }
            if (GetAsyncKeyState('K') & 0x8000) {
                ShipAPI_Backward(targetObject, deltaTime);
            }
            if (GetAsyncKeyState('L') & 0x8000) {
                ShipAPI_TurnRight(targetObject, deltaTime);
            }
            break;
        case CONTROL_ARROWS:
            if (GetAsyncKeyState(VK_UP) & 0x8000) {
                ShipAPI_Forward(targetObject, deltaTime);
            } else {
                ShipAPI_EngineDown(targetObject);
            }
            if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
                ShipAPI_TurnLeft(targetObject, deltaTime);
            }
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
                ShipAPI_Backward(targetObject, deltaTime);
            }
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
                ShipAPI_TurnRight(targetObject, deltaTime);
            }
            break;
        default:
            // 默认情况处理
            break;
    }
}