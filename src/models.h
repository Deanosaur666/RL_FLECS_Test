#ifndef _models
#define _models

#include "headers.h"
#include "main.h"
#include "actors.h"


void DrawModelMatTransform(Model model, Matrix transform, Color tint);
void DrawModelWiresMatTransform(Model model, Matrix transform, Color tint);

Mesh GenMeshPlane2(float width, float length, int resX, int resZ);
Mesh GenMeshInvertedCube(float width, float depth, float height);

extern const Vector3 EXPAND_VECTOR;

#endif