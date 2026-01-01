#ifndef _steering
#define _steering

#include "headers.h"
#include "main.h"
#include "actors.h"

#define COMPASS_RES 8
#define COMPASS_MAX 2

typedef struct ActorCompass {
    Vector2 direction;          // actor's current direction
    float desire[COMPASS_RES];  // actor's desire compass
    float avoid[COMPASS_RES];   // actor's avoid compass
} ActorCompass;

void SteerActor(ActorCompass * compass, Actor actor, Position position);

#endif