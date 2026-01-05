#ifndef _collision
#define _collision

#include "headers.h"
#include "main.h"

#define CCD_TO_RL_VEC3(vec) (Vector3){ (float)vec[0], (float)vec[1], (float)vec[2] }
#define RL_TO_CCD_VEC3(vec) ((ccd_vec3_t){ (ccd_real_t)vec.x, (ccd_real_t)vec.y, (ccd_real_t)vec.z })

typedef struct VertexMesh {
    Vector3 * verts;
    int vertCount;
} VertexMesh;

typedef struct Collision {
    bool hit;
    float depth;
    Vector3 direction;
    Vector3 point;
} Collision;

typedef struct MeshCollider {
    Mesh * mesh;
    BoundingBox box;
    Matrix * transform;
} MeshCollider;

float Vector3MixedProduct(Vector3 v0, Vector3 v1, Vector3 v2);
Vector3 Vector3TripleProduct(Vector3 v0, Vector3 v1, Vector3 v2);
Vector3 * Vector3ArrayTransform(Vector3 * in, int count, Matrix matTransform);
Vector3 * MinkowskiDifference3(Vector3 * a_verts, int a_count, Vector3 * b_verts, int b_count);
void VertexMeshSupport(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *vec);
Collision VertexMeshCollision(VertexMesh a, VertexMesh b);
VertexMesh MeshToVertexMesh(Mesh mesh, Matrix matTransform);
bool BoundingBoxIntersects(BoundingBox a, BoundingBox b);
Collision MeshCollision(MeshCollider a, MeshCollider b);
MeshCollider * GetModelMeshColliders(Model model, Matrix * transform);

#endif