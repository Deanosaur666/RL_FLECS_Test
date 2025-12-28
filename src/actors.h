#include "headers.h"
#include "main.h"

#ifndef _actors
#define _actors

typedef enum  {
    ACTOR_SIZE_POINT,
    ACTOR_SIZE_SMALL,

    ACTOR_SIZE_COUNT
} ACTOR_SIZE;

typedef enum {
    ACTOR_RED,
    ACTOR_YELLOW,
    ACTOR_GREEN,
    ACTOR_PURPLE,
    
    ACTOR_CAT,
    ACTOR_PURSUER,
} ACTOR_TYPE;

extern Vector3 ACTOR_SIZE_VECTORS[ACTOR_SIZE_COUNT];

#define ACTOR_HIT_MARGIN 0.1f

#define ACTOR_SMALL_R 0.25f                         // xy radius
#define ACTOR_SMALL_Ho2 ((14.0f / 16.0f) / 2.0f)    // z radius, height over 2

typedef struct Actor {
    ACTOR_SIZE size;
    ACTOR_TYPE type;
    Vector3 velocity;
} Actor;

#define GRAVITY 9.8f / 360.0f // -9.8f / 60.0f
extern Vector3 gravity;

#define ACTOR_SPEED 0.06f

#define ACTOR_MAX_SPEED 0.06f
#define ACTOR_AIR_MAX_SPEED ACTOR_MAX_SPEED/10.0f
#define ACTOR_MIN_SPEED 0.001f // stop at this point
#define ACTOR_ACCEL ACTOR_MAX_SPEED/5.0f
#define ACTOR_FRICTION ACTOR_ACCEL/2.0f


#define V3toV2(V3) (Vector2){ V3.x, V3.y }
#define V2toV3(V2, z) (Vector3){ V2.x, V2.y, z }

#define PERPL(V2) (Vector2){ -V2.y, V2.x }
#define PERPR(V2) (Vector2){ V2.y, -V2.x }

void ActorPhysics(Actor * actor, Position * position, Vector2 movement);
float MoveActorRayCollision(Actor * actor, Position * position, Vector3 movement, RayCollision c);
float MoveActor(Actor * actor, Position * position, Vector3 hitcore, Vector3 movement, RayCollision * c);
Vector3 GetTiltVector(Vector2 vec, Vector3 normal);

typedef enum {
    SPRITE_RED,
    SPRITE_YELLOW,
    SPRITE_GREEN,
    SPRITE_PURPLE,

    SPRITE_CAT,
    SPRITE_PURSUER,
    
    SPRITE_REDCOIN,
    SPRITE_GREENCOIN,
    SPRITE_YELLOWCOIN,
    SPRITE_PURPLECOIN,

    SPRITE_REDBEAM,
    SPRITE_GREENBEAM,
    SPRITE_YELLOWBEAM,
    SPRITE_PURPLEBEAM,

    SPRITE_REDHIT_1,
    SPRITE_REDHIT_2,
    SPRITE_GREENHIT_1,
    SPRITE_GREENHIT_2,
    SPRITE_YELLOWHIT_1,
    SPRITE_YELLOWHIT_2,
    SPRITE_PURPLEHIT_1,
    SPRITE_PURPLEHIT_2,

    SPRITE_BLOOD_0,
    SPRITE_BLOOD_1,
    SPRITE_BLOOD_2,
    SPRITE_BLOOD_3,
    SPRITE_BLOOD_4,

    SPRITE_COUNT,
} SPRITE;

#endif