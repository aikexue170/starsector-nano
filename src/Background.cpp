#include "Background.h"

Background* Background_Init(ResourceManager* rsm, RenderManager* rdm, const char* name){
    Background* background = new(Background);
    ResourceManager_GetIMAGE(rsm, &background->image, "background", name);
    background->id = RenderManager_AddBackground(rdm, &background->image);
    return background;
}