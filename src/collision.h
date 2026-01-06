#ifndef _collision
#define _collision

#include "headers.h"
#include "main.h"

extern ecs_query_t * q_MeshCollider;
extern ecs_query_t * q_BoxCollider;

#define ECS_COLLIDER_COMPONENTS() \
ECS_COMPONENT(world, MeshCollider); \
ECS_COMPONENT(world, BoxCollider)

#define ECS_COLLIDER_QUERIES() \
q_MeshCollider = ecs_query(world, { \
    .terms = { \
        { ecs_id(MeshCollider) } \
    }, \
    .cache_kind = EcsQueryCacheAll, \
}); \
q_BoxCollider = ecs_query(world, { \
    .terms = { \
        { ecs_id(BoxCollider) } \
    }, \
    .cache_kind = EcsQueryCacheAll, \
})

#define CCD_TO_RL_VEC3(vec) (Vector3){ (float)vec[0], (float)vec[1], (float)vec[2] }
#define RL_TO_CCD_VEC3(vec) ((ccd_vec3_t){ (ccd_real_t)vec.x, (ccd_real_t)vec.y, (ccd_real_t)vec.z })
#define RL_PTR_TO_CCD_VEC3(vec) ((ccd_vec3_t){ (ccd_real_t)vec->x, (ccd_real_t)vec->y, (ccd_real_t)vec->z })

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

// for component system
typedef BoundingBox BoxCollider;
typedef Vector3 PointCollider;

void VertexMeshSupport(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *vec);
Collision VertexMeshCollision(VertexMesh a, VertexMesh b);
VertexMesh MeshToVertexMesh(Mesh mesh, Matrix matTransform);
Collision MeshCollision(MeshCollider a, MeshCollider b);
MeshCollider * GetModelMeshColliders(Model model, Matrix * transform);
MeshCollider GetModelMeshCollider0(Model model, Matrix * transform);

Collision PointMeshCollision(Vector3 p, MeshCollider m);
Collision BoxMeshCollision(BoundingBox box, MeshCollider m);
Collision BoxBoxCollision(BoundingBox b1, BoundingBox b2);
Collision PointBoxCollision(Vector3 p, BoundingBox box);

RayCollision RayToMeshColliders(Ray ray, float distance);
RayCollision RayToBoxColliders(Ray ray, float distance);
RayCollision RayToAnyCollider(Ray ray, float distance);

#endif