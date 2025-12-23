#ifndef STARSECTOR_NANO_BACKGROUND_H
#define STARSECTOR_NANO_BACKGROUND_H
#include "Manager.h"
#include "Vector2f.h"

typedef struct Background {
    IMAGE image;
    Vector2f position;
    Vector2f velocity;
    int id = -1;
}Background;

Background* Background_Init(ResourceManager* rm,
                     RenderManager* rdm,
                     const char* name);

#endif //STARSECTOR_NANO_BACKGROUND_H
