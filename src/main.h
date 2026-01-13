#ifndef _main
#define _main

#include "headers.h"

extern Vector3 up;
extern Vector3 down;
extern Vector3 unit_vector;
extern Camera camera;
extern ecs_world_t * world;
typedef Vector3 Position;

extern Vector3 mouseWorld;

#define q_(suffix) q_##suffix

#endif