#include "headers.h"

#undef FLT_MAX
#define FLT_MAX     340282346638528859811704183484516925440.0f     // Maximum value of a float, from bit pattern 01111111011111111111111111111111

typedef Vector3 Position;

Mesh GenMeshPlane2(float width, float length, int resX, int resZ);

int main () {

	ecs_world_t * world = ecs_init();

	ECS_COMPONENT(world, Vector3);
	ECS_COMPONENT(world, Position);

	ecs_entity_t e = ecs_new(world);
	ecs_set(world, e, Vector3, {0, 0, 0});
	//ecs_set(world, e, Position, {0, 0, 0});

	printf("%b\n", ecs_has(world, e, Vector3));
	printf("%b\n", ecs_has(world, e, Position));


	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(1280, 800, "flecs Test");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 1.8f, 0.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 1.8f, -1.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

	SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

	// models
	Model model_plain = LoadModelFromMesh(GenMeshPlane2(16, 16, 16, 16));

	Image fulltex = LoadImage("sprites.png");
	//Image img_plain = ImageFromImage(fulltex, (Rectangle){ 0, 32, 16, 16 });
	Image img_plain = ImageFromImage(fulltex, (Rectangle){ 16*4, 16*2, 16, 16 });
	Texture2D tex_plain = LoadTextureFromImage(img_plain);
	tex_plain.width = 256;
	tex_plain.height = 256;

	model_plain.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex_plain;
	
	DisableCursor();

	// game loop
	while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
	{
		UpdateCamera(&camera, CAMERA_FREE);
		
		// Draw
        //----------------------------------------------------------------------------------
		BeginDrawing();

			// Setup the back buffer for drawing (clear color and depth buffers)
			ClearBackground(BLACK);

			BeginMode3D(camera);

				DrawModel(model_plain, (Vector3){0}, 1.0f, WHITE);
				//DrawModelWires(model_plain, (Vector3){0}, 1.0f, RED);
               	//DrawGrid(16, 1.0);

			EndMode3D();
		
			DrawFPS(10, 10);
		
		EndDrawing();
        //----------------------------------------------------------------------------------
	}

	// cleanup

	ecs_fini(world);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}

Mesh GenMeshPlane2(float width, float length, int resX, int resZ)
{
    Mesh mesh = { 0 };

    resX++;
    resZ++;

    // Vertices definition
    int vertexCount = resX*resZ; // vertices get reused for the faces

    Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int z = 0; z < resZ; z++)
    {
        // [-length/2, length/2]
        float zPos = ((float)z/(resZ - 1) - 0.5f)*length;
        for (int x = 0; x < resX; x++)
        {
            // [-width/2, width/2]
            float xPos = ((float)x/(resX - 1) - 0.5f)*width;

			float yPos = (float)GetRandomValue(0, 16);
			yPos /= 16.0f;

            vertices[x + z*resX] = (Vector3){ xPos, yPos, zPos };
        }
    }

    // Normals definition
    Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++) normals[n] = (Vector3){ 0.0f, 1.0f, 0.0f };   // Vector3.up;

    // TexCoords definition
    Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
    for (int v = 0; v < resZ; v++)
    {
        for (int u = 0; u < resX; u++)
        {
            texcoords[u + v*resX] = (Vector2){ (float)u, (float)v };
        }
    }

    // Triangles definition (indices)
    int numFaces = (resX - 1)*(resZ - 1);
    int *triangles = (int *)RL_MALLOC(numFaces*6*sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++)
    {
        // Retrieve lower left corner from face ind
        int i = face + face/(resX - 1);

        triangles[t++] = i + resX;
        triangles[t++] = i + 1;
        triangles[t++] = i;

        triangles[t++] = i + resX;
        triangles[t++] = i + resX + 1;
        triangles[t++] = i + 1;
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