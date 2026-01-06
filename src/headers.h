#ifndef _headers
#define _headers

#include "raylib.h"
#include "raymath.h"
#include "flecs.h"
#include "resource_dir.h"
#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "libccd/ccd/ccd.h"

#undef FLT_MAX
#define FLT_MAX     340282346638528859811704183484516925440.0f     // Maximum value of a float, from bit pattern 01111111011111111111111111111111

#define ACTOR_COUNT 1 // 64
#define BLOCK_COUNT 0 // 32

#define DEBUG 1
#define DRAWWIRES 0
#define DRAWBBOX 0
#define DRAWEXMESHSIZE ACTOR_SIZE_SMALL
#define DRAW_SHAPES 0
#define DRAW_COLLIDER_BOXES 0

#define SIGN(x) (x == 0 ? 0 : x < 0 ? -1 : 1)

#define VECTOR_IS_NAN(vector) \
(isnan(vector.x) || isnan(vector.y) || isnan(vector.z))

#define VECTOR_PTR_IS_NAN(vector) \
(isnan(vector->x) || isnan(vector->y) || isnan(vector->z))

#define VECTOR3SIGN(vector) \
(Vector3){ SIGN(vector.x), SIGN(vector.y), SIGN(vector.z)  }

#endif