#include "headers.h"
#include "models.h"
#include "main.h"

ecs_query_t * q_models;

Matrix MatrixFromTransform(ModelTransform t) {
    Matrix matScale = MatrixScale(t.scale.x, t.scale.y, t.scale.z);
    Matrix matRotation = MatrixRotate(t.rotationAxis, t.rotationAngle*DEG2RAD);
    Matrix matTranslation = MatrixTranslate(t.position.x, t.position.y, t.position.z);

    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

    return matTransform;
}

RayCollision RayToModels(Ray ray) {
	RayCollision collision = { 0 };
	collision.distance = FLT_MAX;
	collision.hit = false;

    ecs_iter_t it = ecs_query_iter(world, q_models);

    while (ecs_query_next(&it))  {
        MapModel *models = ecs_field(&it, MapModel, 0);
        ModelTransform *transforms = ecs_field(&it, ModelTransform, 1);

        for(int i = 0; i < it.count; i ++) {
            Model model = models[i].model;
            ModelTransform transform = transforms[i];

            Matrix matTransform = MatrixFromTransform(transform);

            // Check ray collision against bounding box first, before trying the full ray-mesh test
            //RayCollision boxHitInfo = GetRayCollisionBox(ray, GetMeshBoundingBox(model.meshes[0]));
            //if ((boxHitInfo.hit) && (boxHitInfo.distance < collision.distance))
            {
                // Check ray collision against model meshes
                RayCollision meshHitInfo = { 0 };
                for (int m = 0; m < model.meshCount; m++)
                {
                    meshHitInfo = GetRayCollisionMesh(ray, model.meshes[m], matTransform);
                    float hitAngle = Vector3Angle(ray.direction, meshHitInfo.normal)*RAD2DEG;

                    if (meshHitInfo.hit && hitAngle >= 90.0f)
                    {
                        // Save the closest hit mesh
                        if ((!collision.hit) || (collision.distance > meshHitInfo.distance))
                            collision = meshHitInfo;

                        break;  // Stop once one mesh collision is detected, the colliding mesh is m
                    }
                }
            }
        }
    }

	return collision;
}

float GetElevation(float x, float y, float z) {
    Ray ray = { {x, y, z}, down };
    RayCollision collision = RayToModels(ray);

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