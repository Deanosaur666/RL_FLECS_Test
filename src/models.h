#ifndef _models
#define _models

#include "headers.h"
#include "main.h"
#include "actors.h"

DECLARE_LIST(Model);

#define MapModel_(size) MapModel_##size
#define MapBoxes_(size) MapBoxes_##size

typedef Model MapModel;

typedef Model MapModel_(ACTOR_SIZE_POINT);
typedef Model MapModel_(ACTOR_SIZE_SMALL);

typedef struct MapBoxes {
    BoundingBox modelBox;
    BoundingBox * meshBoxes;
} MapBoxes;

typedef MapBoxes MapBoxes_(ACTOR_SIZE_POINT);
typedef MapBoxes MapBoxes_(ACTOR_SIZE_SMALL);

extern ecs_query_t * q_MapModel;
extern ecs_query_t * q_MapModelEx[ACTOR_SIZE_COUNT];

#define ecs_set_maps(entity, mapsPTR) \
ecs_set_ptr(world, entity, MapModel, &mapsPTR.original); \
\
ecs_set_ptr(world, entity, MapModel_(ACTOR_SIZE_POINT), &mapsPTR.expanded[ACTOR_SIZE_POINT]); \
MapBoxes mapsPTR##_POINT = GetMapBoxes(mapsPTR.expanded[ACTOR_SIZE_POINT]); \
ecs_set_ptr(world, entity, MapBoxes_(ACTOR_SIZE_POINT), &mapsPTR##_POINT); \
\
ecs_set_ptr(world, entity, MapModel_(ACTOR_SIZE_SMALL), &mapsPTR.expanded[ACTOR_SIZE_SMALL]); \
MapBoxes mapsPTR##_SMALL = GetMapBoxes(mapsPTR.expanded[ACTOR_SIZE_SMALL]); \
ecs_set_ptr(world, entity, MapBoxes_(ACTOR_SIZE_SMALL), &mapsPTR##_SMALL)

#define ECS_MAP_COMPONENTS() \
ECS_COMPONENT(world, MapModel); \
ECS_COMPONENT(world, MapModel_ACTOR_SIZE_POINT); \
ECS_COMPONENT(world, MapModel_ACTOR_SIZE_SMALL); \
ECS_COMPONENT(world, MapBoxes_ACTOR_SIZE_POINT); \
ECS_COMPONENT(world, MapBoxes_ACTOR_SIZE_SMALL)

#define ECS_MAP_QUERIES() \
q_MapModel = ecs_query(world, { \
    .terms = { \
        { ecs_id(MapModel) }, { ecs_id(ModelTransform) } \
    }, \
    .cache_kind = EcsQueryCacheAll, \
}); \
\
q_MapModelEx[ACTOR_SIZE_POINT] = ecs_query(world, { \
    .terms = { \
        { ecs_id(MapModel_ACTOR_SIZE_POINT) }, { ecs_id(ModelTransform) }, { ecs_id(MapBoxes_ACTOR_SIZE_POINT) } \
    }, \
    .cache_kind = EcsQueryCacheAll, \
}); \
\
q_MapModelEx[ACTOR_SIZE_SMALL] = ecs_query(world, { \
    .terms = { \
        { ecs_id(MapModel_ACTOR_SIZE_SMALL) }, { ecs_id(ModelTransform) }, { ecs_id(MapBoxes_ACTOR_SIZE_SMALL) } \
    }, \
    .cache_kind = EcsQueryCacheAll, \
}); \


typedef struct MapModelCollection {
    Model original;
    Model expanded[ACTOR_SIZE_COUNT];
} MapModelCollection;

typedef struct ModelTransform {
    Position position;
    Vector3 scale;
    Vector3 rotationAxis;
    float rotationAngle;
} ModelTransform;

Matrix MatrixFromTransform(ModelTransform t);
Mesh GenMeshPlane2(float width, float length, int resX, int resZ);
Mesh GenMeshInvertedCube(float width, float depth, float height);
Mesh ExpandMesh(Mesh original, Vector3 expandScale);
Model ExpandModel(Model original,Vector3 expandScale);

MapModelCollection MakeMapModelCollection(Model original, LIST_(Model) * modelList);

MapBoxes GetMapBoxes(Model model);

RayCollision RayToModels(Ray ray, ACTOR_SIZE size);

float GetElevation(float x, float y, float z, ACTOR_SIZE size);

extern const Vector3 EXPAND_VECTOR;

#endif