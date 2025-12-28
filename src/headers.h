#ifndef _headers
#define _headers

#include "raylib.h"
#include "raymath.h"
#include "flecs.h"
#include "resource_dir.h"
#include <stdio.h>
#include <stdlib.h>
#include "list.h"

#undef FLT_MAX
#define FLT_MAX     340282346638528859811704183484516925440.0f     // Maximum value of a float, from bit pattern 01111111011111111111111111111111

#define ACTOR_COUNT 64
#define BLOCK_COUNT 0 //32

#define DEBUG 1
#define DRAWWIRES 1
#define DRAWBBOX 0
#define DRAWEXMESHSIZE ACTOR_SIZE_SMALL

#endif