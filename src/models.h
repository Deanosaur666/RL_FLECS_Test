#ifndef _models
#define _models

#include "headers.h"
#include "main.h"
#include "actors.h"

DECLARE_PLIST(Model);

typedef struct MapModel {
    Model model;
    Model expanded[ACTOR_SIZE_COUNT];
} MapModel;

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

MapModel MakeMapModel(Model original, PLIST_(Model) * modelList);

RayCollision RayToModels(Ray ray, ACTOR_SIZE size);

float GetElevation(float x, float y, float z, ACTOR_SIZE size);

extern ecs_query_t * q_models;
extern const Vector3 EXPAND_VECTOR;

#endif