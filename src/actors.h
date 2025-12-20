#include "headers.h"
#include "main.h"

Position MoveVector(Position position, Vector3 movement, float hitRadius, RayCollision * hit);
Position MoveAndSlide(Position position, Vector3 movement, float hitRadius);

typedef enum {
    SPRITE_RED,
    SPRITE_YELLOW,
    SPRITE_GREEN,
    SPRITE_PURPLE,
    
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

    SPRITE_CAT,
    SPRITE_PURSUER,
    SPRITE_BLOOD_0,
    SPRITE_BLOOD_1,
    SPRITE_BLOOD_2,
    SPRITE_BLOOD_3,
    SPRITE_BLOOD_4,

    SPRITE_COUNT,
} SPRITE;

typedef enum {
    ACTOR_RED,
    ACTOR_YELLOW,
    ACTOR_GREEN,
    ACTOR_PURPLE,
    ACTOR_CAT,
    ACTOR_PURSUER,
} ACTOR_TYPE;