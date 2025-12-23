#ifndef STARSECTOR_NANO_TACTICALMAP_H
#define STARSECTOR_NANO_TACTICALMAP_H

#include <graphics.h>
#include <stdbool.h>
#include "GlobalCombatData.h"

typedef struct CombatDataPool CombatDataPool;

void TacticalMap_Init();
bool TacticalMap_Update();
void TacticalMap_Render(ResourceManager* rm, Camera* camera, ShipAPI* player, const CombatDataPool* pool);

#endif //STARSECTOR_NANO_TACTICALMAP_H
