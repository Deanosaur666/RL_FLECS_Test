#include "boymath.h"
#include "headers.h"

float Vector3MixedProduct(Vector3 v0, Vector3 v1, Vector3 v2) {
    return Vector3DotProduct(Vector3CrossProduct(v0, v1), v2);
}

Vector3 Vector3TripleProduct(Vector3 v0, Vector3 v1, Vector3 v2) {
    return Vector3CrossProduct(Vector3CrossProduct(v0, v1), v2);
}

Vector3 * Vector3ArrayTransform(Vector3 * in, int count, Matrix matTransform) {
    Vector3 * out = (Vector3 *)malloc(sizeof(Vector3) * count);
    for(int i = 0; i < count; i ++) {
        out[i] = Vector3Transform(in[i], matTransform);
    }

    return out;
}

bool BoundingBoxIntersects(BoundingBox a, BoundingBox b) {
    return  (a.min.x <= b.max.x && a.max.x >= b.min.x) && 
            (a.min.y <= b.max.y && a.max.y >= b.min.y) && 
            (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

bool BoundingBoxContains(BoundingBox b, Vector3 point) {
    return  (point.x <= b.max.x && point.x >= b.min.x) &&
            (point.y <= b.max.y && point.y >= b.min.y) &&
            (point.z <= b.max.z && point.z >= b.min.z);
}

BoundingBox BoundingBoxAdd(BoundingBox b, Vector3 v) {
    b.min = Vector3Add(b.min, v);
    b.max = Vector3Add(b.max, v);
    return b;
}

BoundingBox TransformBoundingBox(BoundingBox box, Matrix matTransform) {
    
    float west = box.min.x;
    float east = box.max.x;
    float south = box.min.y;
    float north = box.max.y;
    float bottom = box.min.z;
    float top = box.max.z;

    Vector3 eightCorners[] = {
        { west, south, bottom },
        { west, south, top },
        { west, north, bottom },
        { west, north, top },
        { east, south, bottom },
        { east, south, top },
        { east, north, bottom },
        { east, north, top }
    };

    Vector3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
    Vector3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for(int i = 0; i < 8; i ++) {
        Vector3 trans = Vector3Transform(eightCorners[i], matTransform);
        min = Vector3Min(min, trans);
        max = Vector3Max(max, trans);
    }
    
    box.min = min;
    box.max = max;

    return box;
}

Vector3 ClipVector(Vector3 vec, Vector3 normal) {
    float backoff = Vector3DotProduct(vec, normal);
    Vector3 eject = Vector3Scale(normal, backoff);
    return Vector3Subtract(vec, eject);
}