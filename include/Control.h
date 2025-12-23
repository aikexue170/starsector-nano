#ifndef STARSECTOR_NANO_CONTROL_H
#define STARSECTOR_NANO_CONTROL_H

#include "ShipAPI.h"
#include <winuser.h>

typedef enum {
    CONTROL_WASD,
    CONTROL_IJKL,
    CONTROL_ARROWS
} ControlType;

void Control_Handler(ControlType controlType, ShipAPI* targetObject, float deltaTime);


#endif //STARSECTOR_NANO_CONTROL_H
