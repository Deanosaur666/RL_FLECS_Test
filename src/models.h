#ifndef _models
#define _models

#include "headers.h"
#include "main.h"

typedef struct MapModel {
    Model model;
} MapModel;

typedef struct ModelTransform {
    Position position;
    Vector3 scale;
    Vector3 rotationAxis;
    float rotationAngle;
} ModelTransform;

Matrix MatrixFromTransform(ModelTransform t);
Mesh GenMeshPlane2(float width, float length, int resX, int resZ);
RayCollision RayToModels(Ray ray);

Mesh GenMeshPlane2(float width, float length, int resX, int resZ);
float GetElevation(float x, float y, float z);

extern ecs_query_t * q_models;

#endif