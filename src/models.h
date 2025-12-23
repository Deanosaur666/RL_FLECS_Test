#ifndef _models
#define _models

#include "headers.h"
#include "main.h"
#include "actors.h"

DECLARE_PLIST(Model);

#define MapModel_(size) MapModel_##size

typedef Model MapModel;

typedef Model MapModel_(ACTOR_SIZE_POINT);
typedef Model MapModel_(ACTOR_SIZE_SMALL);

extern ecs_query_t * q_MapModel;
extern ecs_query_t * q_MapModelEx[ACTOR_SIZE_COUNT];

#define ecs_set_maps(entity, mapsPTR) \
ecs_set_ptr(world, entity, MapModel, &mapsPTR.original); \
ecs_set_ptr(world, entity, MapModel_(ACTOR_SIZE_POINT), &mapsPTR.expanded[ACTOR_SIZE_POINT]); \
ecs_set_ptr(world, entity, MapModel_(ACTOR_SIZE_SMALL), &mapsPTR.expanded[ACTOR_SIZE_SMALL])

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

MapModelCollection MakeMapModelCollection(Model original, PLIST_(Model) * modelList);

RayCollision RayToModels(Ray ray, ACTOR_SIZE size);

float GetElevation(float x, float y, float z, ACTOR_SIZE size);

extern const Vector3 EXPAND_VECTOR;

#endif