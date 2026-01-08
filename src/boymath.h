#ifndef _boymath
#define _boymath

#include "headers.h"

float Vector3MixedProduct(Vector3 v0, Vector3 v1, Vector3 v2);
Vector3 Vector3TripleProduct(Vector3 v0, Vector3 v1, Vector3 v2);
Vector3 * Vector3ArrayTransform(Vector3 * in, int count, Matrix matTransform);

bool BoundingBoxIntersects(BoundingBox a, BoundingBox b);
bool BoundingBoxContains(BoundingBox b, Vector3 point);
BoundingBox BoundingBoxAdd(BoundingBox b, Vector3 v);
BoundingBox TransformBoundingBox(BoundingBox box, Matrix matTransform);

Vector3 ClipVector(Vector3 vec, Vector3 normal);

#endif