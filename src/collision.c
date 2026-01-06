#include "collision.h"
#include "headers.h"
#include "main.h"
#include "models.h"

ecs_query_t * q_MeshCollider;
ecs_query_t * q_BoxCollider;

void VertexMeshSupport(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *vec) {
    VertexMesh * meshPtr = (VertexMesh *)obj;
    Vector3 direction = CCD_TO_RL_VEC3(dir->v);

    int best_index = 0;
    float best_match = Vector3DotProduct(direction, meshPtr->verts[0]);
    for(int i = 1; i < meshPtr->vertCount; i ++) {
        float dot = Vector3DotProduct(direction, meshPtr->verts[i]);
        if(dot > best_match) {
            best_index = i;
            best_match = dot;
        }
    }

    Vector3 support = meshPtr->verts[best_index];

    *vec = RL_TO_CCD_VEC3(support);
}

Collision VertexMeshCollision(VertexMesh a, VertexMesh b) {
    ccd_t ccd;
    CCD_INIT(&ccd);

    ccd.support1 = VertexMeshSupport;
    ccd.support2 = VertexMeshSupport;
    ccd.max_iterations = 100;

    ccd_real_t depth = 0;
    ccd_vec3_t dir, pos;
    int intersect = ccdGJKPenetration(&b, &a, &ccd, &depth, &dir, &pos);

    return (Collision){ intersect == 0, (float)depth, CCD_TO_RL_VEC3(dir.v), CCD_TO_RL_VEC3(pos.v) };
}

VertexMesh MeshToVertexMesh(Mesh mesh, Matrix matTransform) {
    VertexMesh vMesh = { 0 };
    vMesh.vertCount = mesh.vertexCount;
    // receiver must free these
    vMesh.verts = Vector3ArrayTransform((Vector3 *)mesh.vertices, mesh.vertexCount, matTransform);
    return vMesh;
}

Collision MeshCollision(MeshCollider a, MeshCollider b) {
    Matrix atransform, btransform;
    if(a.transform == NULL)
        atransform = MatrixIdentity();
    else
        atransform = *a.transform;
    if(b.transform == NULL)
        btransform = MatrixIdentity();
    else
        btransform = *b.transform;
        
    BoundingBox abox = TransformBoundingBox(a.box, atransform);
    BoundingBox bbox = TransformBoundingBox(b.box, btransform);

    if(!BoundingBoxIntersects(abox, bbox)) {
        return (Collision){ false };
    }

    VertexMesh va = MeshToVertexMesh(*a.mesh, atransform);
    VertexMesh vb = MeshToVertexMesh(*b.mesh, btransform);

    Collision c = VertexMeshCollision(va, vb);
    free(va.verts);
    free(vb.verts);

    return c;
}

MeshCollider * GetModelMeshColliders(Model model, Matrix * transform) {
    MeshCollider * colliders = malloc(model.meshCount * sizeof(MeshCollider));
    for(int i = 0; i < model.meshCount; i ++) {
        colliders[i] = (MeshCollider){ &model.meshes[i], GetMeshBoundingBox(model.meshes[i]), transform };
    }
    return colliders;
}

MeshCollider GetModelMeshCollider0(Model model, Matrix * transform) {
    return (MeshCollider){ &model.meshes[0], GetMeshBoundingBox(model.meshes[0]), transform };
}

void PointSupport(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *vec) {
    Vector3 * pointPTR = (Vector3 *)obj;

    *vec = RL_PTR_TO_CCD_VEC3(pointPTR);
}

Collision PointVertexMeshCollision(Vector3 p, VertexMesh vm) {
    ccd_t ccd;
    CCD_INIT(&ccd);

    ccd.support1 = VertexMeshSupport;
    ccd.support2 = PointSupport;
    ccd.max_iterations = 100;

    ccd_real_t depth = 0;
    ccd_vec3_t dir, pos;
    int intersect = ccdGJKPenetration(&vm, &p, &ccd, &depth, &dir, &pos);

    return (Collision){ intersect == 0, (float)depth, CCD_TO_RL_VEC3(dir.v), CCD_TO_RL_VEC3(pos.v) };
}

Collision PointMeshCollision(Vector3 p, MeshCollider m) {
    Matrix mtransform;
    if(m.transform == NULL)
        mtransform = MatrixIdentity();
    else
        mtransform = *m.transform;
        
    BoundingBox mbox = TransformBoundingBox(m.box, mtransform);

    if(!BoundingBoxContains(mbox, p)) {
        return (Collision){ false };
    }

    VertexMesh vm = MeshToVertexMesh(*m.mesh, mtransform);

    Collision c = PointVertexMeshCollision(p, vm);

    free(vm.verts);

    return c;
}

void BoundingBoxSupport(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *vec) {
    BoundingBox * bboxPTR = (BoundingBox *)obj;

    Vector3 direction = CCD_TO_RL_VEC3(dir->v);
    Vector3 dirSign = VECTOR3SIGN(direction);

    Vector3 halfsize = Vector3Scale(Vector3Subtract(bboxPTR->max, bboxPTR->min), 0.5f);
    Vector3 center = Vector3Add(bboxPTR->min, halfsize);

    Vector3 support = Vector3Add(center, Vector3Multiply(dirSign, halfsize));

    *vec = RL_TO_CCD_VEC3(support);
}

Collision BoxVertexMeshCollision(BoundingBox box, VertexMesh vm) {
    ccd_t ccd;
    CCD_INIT(&ccd);

    ccd.support1 = VertexMeshSupport;
    ccd.support2 = BoundingBoxSupport;
    ccd.max_iterations = 100;

    ccd_real_t depth = 0;
    ccd_vec3_t dir, pos;
    int intersect = ccdGJKPenetration(&vm, &box, &ccd, &depth, &dir, &pos);

    return (Collision){ intersect == 0, (float)depth, CCD_TO_RL_VEC3(dir.v), CCD_TO_RL_VEC3(pos.v) };
}

Collision BoxMeshCollision(BoundingBox box, MeshCollider m) {
    Matrix mtransform;
    if(m.transform == NULL)
        mtransform = MatrixIdentity();
    else
        mtransform = *m.transform;
        
    BoundingBox mbox = TransformBoundingBox(m.box, mtransform);

    if(!BoundingBoxIntersects(box, mbox)) {
        return (Collision){ false };
    }

    VertexMesh vm = MeshToVertexMesh(*m.mesh, mtransform);

    Collision c = BoxVertexMeshCollision(box, vm);

    free(vm.verts);

    return c;
}

Collision BoxBoxCollision(BoundingBox b1, BoundingBox b2) {

    if(!BoundingBoxIntersects(b1, b2)) {
        return (Collision){ false };
    }

    ccd_t ccd;
    CCD_INIT(&ccd);

    ccd.support1 = BoundingBoxSupport;
    ccd.support2 = BoundingBoxSupport;
    ccd.max_iterations = 100;

    ccd_real_t depth = 0;
    ccd_vec3_t dir, pos;
    int intersect = ccdGJKPenetration(&b1, &b2, &ccd, &depth, &dir, &pos);

    return (Collision){ intersect == 0, (float)depth, CCD_TO_RL_VEC3(dir.v), CCD_TO_RL_VEC3(pos.v) };
}

Collision PointBoxCollision(Vector3 p, BoundingBox box) {

    if(!BoundingBoxContains(box, p)) {
        return (Collision){ false };
    }

    ccd_t ccd;
    CCD_INIT(&ccd);

    ccd.support1 = BoundingBoxSupport;
    ccd.support2 = PointSupport;
    ccd.max_iterations = 100;

    ccd_real_t depth = 0;
    ccd_vec3_t dir, pos;
    int intersect = ccdGJKPenetration(&box, &p, &ccd, &depth, &dir, &pos);

    return (Collision){ intersect == 0, (float)depth, CCD_TO_RL_VEC3(dir.v), CCD_TO_RL_VEC3(pos.v) };
}

RayCollision RayToMeshColliders(Ray ray, float distance) {
	RayCollision collision = { 0 };
	collision.distance = FLT_MAX;
	collision.hit = false;

    ecs_iter_t it = ecs_query_iter(world, q_MeshCollider);

    while (ecs_query_next(&it))  {
        MeshCollider * colliders = ecs_field(&it, MeshCollider, 0);

        for(int i = 0; i < it.count; i ++) {
            MeshCollider collider = colliders[i];
            BoundingBox box = TransformBoundingBox(collider.box, *collider.transform);

            RayCollision boxHitInfo = GetRayCollisionBox(ray, box);
            if ((boxHitInfo.hit)) {
                // Check ray collision against model meshes
                RayCollision meshHitInfo = { 0 };

                meshHitInfo = GetRayCollisionMesh(ray, *collider.mesh, *collider.transform);
                float hitAngle = Vector3Angle(ray.direction, meshHitInfo.normal)*RAD2DEG;

                if (meshHitInfo.hit && hitAngle >= 90.0f && (meshHitInfo.distance < collision.distance) && (meshHitInfo.distance < distance))
                {
                    collision = meshHitInfo;
                }
            }
        }
    }

	return collision;
}

RayCollision RayToBoxColliders(Ray ray, float distance) {
	RayCollision collision = { 0 };
	collision.distance = FLT_MAX;
	collision.hit = false;

    ecs_iter_t it = ecs_query_iter(world, q_BoxCollider);

    while (ecs_query_next(&it))  {
        BoxCollider * colliders = ecs_field(&it, BoxCollider, 0);

        for(int i = 0; i < it.count; i ++) {
            BoxCollider box = colliders[i];

            RayCollision boxHitInfo = GetRayCollisionBox(ray, box);
            if (boxHitInfo.hit && boxHitInfo.distance < collision.distance && boxHitInfo.distance < distance) {
                collision = boxHitInfo;
            }
        }
    }

	return collision;
}

RayCollision RayToAnyCollider(Ray ray, float distance) {
    RayCollision meshHit = RayToMeshColliders(ray, distance);
    RayCollision boxHit = RayToBoxColliders(ray, distance);

    if(meshHit.hit && (!boxHit.hit || meshHit.distance < boxHit.distance))
        return meshHit;
    else
        return boxHit;
}