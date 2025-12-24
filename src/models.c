#include "headers.h"
#include "models.h"
#include "main.h"

ecs_query_t * q_MapModel;
ecs_query_t * q_MapModelEx[ACTOR_SIZE_COUNT];

Matrix MatrixFromTransform(ModelTransform t) {
    Matrix matScale = MatrixScale(t.scale.x, t.scale.y, t.scale.z);
    Matrix matRotation = MatrixRotate(t.rotationAxis, t.rotationAngle*DEG2RAD);
    Matrix matTranslation = MatrixTranslate(t.position.x, t.position.y, t.position.z);

    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

    return matTransform;
}

RayCollision RayToModels(Ray ray, ACTOR_SIZE size, float distance) {
	RayCollision collision = { 0 };
	collision.distance = FLT_MAX;
	collision.hit = false;

    ecs_iter_t it = ecs_query_iter(world, q_MapModelEx[size]);
    //ecs_iter_t it = ecs_query_iter(world, q_MapModel);

    while (ecs_query_next(&it))  {
        MapModel *models = ecs_field(&it, MapModel, 0);
        ModelTransform *transforms = ecs_field(&it, ModelTransform, 1);
        MapBoxes *boxes = ecs_field(&it, MapBoxes, 2);

        for(int i = 0; i < it.count; i ++) {
            Model model = models[i];
            ModelTransform transform = transforms[i];

            MapBoxes modelBoxes = boxes[i];

            BoundingBox box = modelBoxes.modelBox;

            Matrix matTransform = MatrixFromTransform(transform);
            // Check ray collision against bounding box first, before trying the full ray-mesh test
            box.min = Vector3Transform(box.min, matTransform);
            box.max = Vector3Transform(box.max, matTransform);

            RayCollision boxHitInfo = GetRayCollisionBox(ray, box);
            if ((boxHitInfo.hit)) {// && (boxHitInfo.distance < distance) && (boxHitInfo.distance < collision.distance))
                // Check ray collision against model meshes
                RayCollision meshHitInfo = { 0 };
                for (int m = 0; m < model.meshCount; m++)
                {  
                    if(m != 0) {
                        box = modelBoxes.meshBoxes[m];
                        box.min = Vector3Transform(box.min, matTransform);
                        box.max = Vector3Transform(box.max, matTransform);
                        boxHitInfo = GetRayCollisionBox(ray, box);
                    }
                    if ((boxHitInfo.hit)) {// && (boxHitInfo.distance < distance) && (boxHitInfo.distance < collision.distance)) {

                        meshHitInfo = GetRayCollisionMesh(ray, model.meshes[m], matTransform);
                        float hitAngle = Vector3Angle(ray.direction, meshHitInfo.normal)*RAD2DEG;

                        if (meshHitInfo.hit && hitAngle >= 90.0f && (meshHitInfo.distance < distance) && (meshHitInfo.distance < collision.distance))
                        {
                            collision = meshHitInfo;
                        }
                    }
                }
            }
        }
    }

	return collision;
}

float GetElevation(float x, float y, float z, ACTOR_SIZE size) {
    Ray ray = { {x, y, z}, down };
    RayCollision collision = RayToModels(ray, size, FLT_MAX);

    if(collision.hit) {
        return collision.point.z;
    }

    return FLT_MAX;
}

Mesh GenMeshPlane2(float width, float length, int resX, int resY) {
    
    Mesh mesh = { 0 };

    resX++;
    resY++;

    // Vertices definition
    int vertexCount = resX*resY; // vertices get reused for the faces

    Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int y = 0; y < resY; y++) {
        // [-length/2, length/2]
        float yPos = ((float)y/(resY - 1) - 0.5f)*length;
        for (int x = 0; x < resX; x++) {
            // [-width/2, width/2]
            float xPos = ((float)x/(resX - 1) - 0.5f)*width;

			float zPos = (float)GetRandomValue(0, 16);
			zPos /= 16.0f;

            vertices[x + y*resX] = (Vector3){ xPos, yPos, zPos };
        }
    }

    // Normals definition
    Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++)
        normals[n] = up;

    // TexCoords definition
    Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
    for (int v = 0; v < resY; v++) {
        for (int u = 0; u < resX; u++) {
            texcoords[u + v*resX] = (Vector2){ (float)u, (float)v };
        }
    }

    // Triangles definition (indices)
    int numFaces = (resX - 1)*(resY - 1);
    int *triangles = (int *)RL_MALLOC(numFaces*6*sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++)
    {
        // Retrieve lower left corner from face ind
        int i = face + face/(resX - 1);

        triangles[t++] = i + resX;
        triangles[t++] = i;
        triangles[t++] = i + 1;

        triangles[t++] = i + resX;
        triangles[t++] = i + 1;
        triangles[t++] = i + resX + 1;
    }

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numFaces*2;
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3*i] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2*i] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3*i] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }

    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount*3; i++) mesh.indices[i] = triangles[i];

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);
    RL_FREE(triangles);

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);
    return mesh;
}

Mesh GenMeshInvertedCube(float width, float depth, float height) {
    Mesh mesh = { 0 };

    float vertices[] = {
        -width/2, -depth/2, height/2,
        width/2, -depth/2, height/2,
        width/2, depth/2, height/2,
        -width/2, depth/2, height/2,
        -width/2, -depth/2, -height/2,
        -width/2, depth/2, -height/2,
        width/2, depth/2, -height/2,
        width/2, -depth/2, -height/2,
        -width/2, depth/2, -height/2,
        -width/2, depth/2, height/2,
        width/2, depth/2, height/2,
        width/2, depth/2, -height/2,
        -width/2, -depth/2, -height/2,
        width/2, -depth/2, -height/2,
        width/2, -depth/2, height/2,
        -width/2, -depth/2, height/2,
        width/2, -depth/2, -height/2,
        width/2, depth/2, -height/2,
        width/2, depth/2, height/2,
        width/2, -depth/2, height/2,
        -width/2, -depth/2, -height/2,
        -width/2, -depth/2, height/2,
        -width/2, depth/2, height/2,
        -width/2, depth/2, -height/2
    };

    float texcoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    float normals[] = {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f
    };



    mesh.vertices = (float *)RL_MALLOC(24*3*sizeof(float));
    memcpy(mesh.vertices, vertices, 24*3*sizeof(float));

    mesh.texcoords = (float *)RL_MALLOC(24*2*sizeof(float));
    memcpy(mesh.texcoords, texcoords, 24*2*sizeof(float));

    mesh.normals = (float *)RL_MALLOC(24*3*sizeof(float));
    memcpy(mesh.normals, normals, 24*3*sizeof(float));

    mesh.indices = (unsigned short *)RL_MALLOC(36*sizeof(unsigned short));

    int k = 0;

    // Indices can be initialized right now
    for (int i = 0; i < 36; i += 6) {
        /*
        mesh.indices[i] = 4*k;
        mesh.indices[i + 1] = 4*k + 1;
        mesh.indices[i + 2] = 4*k + 2;
        mesh.indices[i + 3] = 4*k;
        mesh.indices[i + 4] = 4*k + 2;
        mesh.indices[i + 5] = 4*k + 3;
        */

        mesh.indices[i] = 4*k;
        mesh.indices[i + 1] = 4*k + 2;
        mesh.indices[i + 2] = 4*k + 1;
        mesh.indices[i + 3] = 4*k;
        mesh.indices[i + 4] = 4*k + 3;
        mesh.indices[i + 5] = 4*k + 2;

        k++;
    }

    mesh.vertexCount = 24;
    mesh.triangleCount = 12;

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

Mesh ExpandMesh(Mesh original, Vector3 expandScale) {
    Mesh mesh = { 0 };
    mesh.vertexCount = original.vertexCount;
    mesh.triangleCount = original.triangleCount;

    // copy vertices (these will be modified)
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    memcpy(mesh.vertices, original.vertices, mesh.vertexCount*3*sizeof(float));

    // copy other stuff?
    if(original.normals) {
        mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
        memcpy(mesh.normals, original.normals, mesh.vertexCount*3*sizeof(float));
    }

    if(original.indices) {
        mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));
        memcpy(mesh.indices, original.indices, mesh.triangleCount*3*sizeof(unsigned short));
    }

    // find duplicate vertices
    Vector3 *vertdata = (Vector3 *)mesh.vertices;

    // we consider the FIRST vertex at a position to be the original
    int vertexDuplicates[mesh.vertexCount];

    for(int i = 0; i < mesh.vertexCount; i ++) {
        vertexDuplicates[i] = -1; // not a duplicate
        Vector3 vert = vertdata[i];
        for(int j = 0; j < i; j ++) {
            Vector3 vert2 = vertdata[j];
            if(Vector3Equals(vert, vert2)) {
                vertexDuplicates[i] = j;
                printf("Duplicate vertices: %d, %d\n", i, j);
                break;
            }
        }
    }


    // expansion
    Vector3 pushVectors[mesh.vertexCount];
    memset(pushVectors, 0, mesh.vertexCount * sizeof(Vector3));
    Vector3 pushScale[mesh.vertexCount];
    memset(pushScale, 0, mesh.vertexCount * sizeof(Vector3));

    for(int i = 0; i < mesh.triangleCount; i ++) {
        int ai, bi, ci;
        Vector3 *a, *b, *c, edge1, edge2, normal, expand;

        if (mesh.indices)
        {
            ai = mesh.indices[i*3 + 0];
            bi = mesh.indices[i*3 + 1];
            ci = mesh.indices[i*3 + 2];
        }
        else
        {
            ai = i*3 + 0;
            bi = i*3 + 1;
            ci = i*3 + 2;
        }

        a = &vertdata[ai];
        b = &vertdata[bi];
        c = &vertdata[ci];

        edge1 = Vector3Subtract(*b, *a);
        edge2 = Vector3Subtract(*c, *a);
        normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));

        expand = Vector3Multiply(normal, expandScale);

        // if this vertex is a duplicate, we instead go to the source
        ai = vertexDuplicates[ai] != -1 ? vertexDuplicates[ai] : ai;
        bi = vertexDuplicates[bi] != -1 ? vertexDuplicates[bi] : bi;
        ci = vertexDuplicates[ci] != -1 ? vertexDuplicates[ci] : ci;

        pushVectors[ai] = Vector3Add(pushVectors[ai], expand);
        pushVectors[bi] = Vector3Add(pushVectors[bi], expand);
        pushVectors[ci] = Vector3Add(pushVectors[ci], expand);

        pushScale[ai] = Vector3Add(pushScale[ai], normal);
        pushScale[bi] = Vector3Add(pushScale[bi], normal);
        pushScale[ci] = Vector3Add(pushScale[ci], normal);
    }

    for(int i = 0; i < mesh.vertexCount; i ++) {
        int vi = vertexDuplicates[i] != -1 ? vertexDuplicates[i] : i;
        
        Vector3 * vert = &vertdata[i];

        Vector3 expand = pushVectors[vi];
        Vector3 scale = pushScale[vi];
        //expand = Vector3Scale(expand, 1.0f / (float)count);
        //expand = Vector3Divide(expand, scale);
        expand = (Vector3){ expand.x / fabsf(scale.x), expand.y / fabsf(scale.y), expand.z / fabsf(scale.z) };

        *vert = Vector3Add(*vert, expand);
    }

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

Model ExpandModel(Model original,Vector3 expandScale) {
    Model model = { 0 };
    model.transform = original.transform;
    model.meshCount = original.meshCount;
    model.meshes = (Mesh *)RL_CALLOC(model.meshCount, sizeof(Mesh));

    // expand meshes
    for(int i = 0; i < model.meshCount; i ++) {
        model.meshes[i] = ExpandMesh(original.meshes[i], expandScale);
    }

    // materials because IDK if it will crash without them
    model.materialCount = 0;

    return model;

    model.materialCount = 1;
    model.materials = (Material *)RL_CALLOC(model.materialCount, sizeof(Material));
    model.materials[0] = LoadMaterialDefault();

    model.meshMaterial = (int *)RL_CALLOC(model.meshCount, sizeof(int));
    model.meshMaterial[0] = 0;  // First material index

    return model;
}

MapModelCollection MakeMapModelCollection(Model original, LIST_(Model) * modelListPtr) {
    MapModelCollection mm = { original };
    LIST_(Model) modelList = *modelListPtr;
    
    LIST_ADD(modelList, original);

    for(int i = 0; i < ACTOR_SIZE_COUNT; i ++) {
        Model expanded = ExpandModel(original, ACTOR_SIZE_VECTORS[i]);
        mm.expanded[i] = expanded;
        LIST_ADD(modelList, expanded);
    }

    *modelListPtr = modelList;

    return mm;
}

MapBoxes GetMapBoxes(Model model) {
    MapBoxes boxes = { 0 };
    // REMEMBER TO FREE
    boxes.meshBoxes = (BoundingBox *)malloc(model.meshCount * sizeof(BoundingBox));

    Vector3 minVertex = { 0 };
    Vector3 maxVertex = { 0 };

    for(int i = 0; i < model.meshCount; i ++) {
        boxes.meshBoxes[i] = GetMeshBoundingBox(model.meshes[i]);

        minVertex = Vector3Min(minVertex, boxes.meshBoxes[i].min);
        maxVertex = Vector3Max(maxVertex, boxes.meshBoxes[i].max);
    }
    BoundingBox modelBox = { 0 };
    modelBox.min = minVertex;
    modelBox.max = maxVertex;
    
    boxes.modelBox = modelBox;

    return boxes;
}