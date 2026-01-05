#include "collision.h"
#include "headers.h"
#include "main.h"
#include "models.h"

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

Vector3 * MinkowskiDifference3(Vector3 * a_verts, int a_count, Vector3 * b_verts, int b_count) {
    int diff_count = a_count * b_count;
    // receiver must free this
    Vector3 * diff_verts = (Vector3 *)malloc(sizeof(Vector3) * diff_count);
    int d = 0;
    for(int a = 0; a < a_count; a ++) {
        for(int b = 0; b < b_count; b ++) {
            diff_verts[d] = Vector3Subtract(b_verts[b], a_verts[a]);
            d ++;
        }
    }

    return diff_verts;
}

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
    int intersect = ccdGJKPenetration(&a, &b, &ccd, &depth, &dir, &pos);

    return (Collision){ intersect == 0, (float)depth, CCD_TO_RL_VEC3(dir.v), CCD_TO_RL_VEC3(pos.v) };
}

VertexMesh MeshToVertexMesh(Mesh mesh, Matrix matTransform) {
    VertexMesh vMesh = { 0 };
    vMesh.vertCount = mesh.vertexCount;
    // receiver must free these
    vMesh.verts = Vector3ArrayTransform((Vector3 *)mesh.vertices, mesh.vertexCount, matTransform);
    return vMesh;
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