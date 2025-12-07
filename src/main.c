#include "headers.h"

#undef FLT_MAX
#define FLT_MAX     340282346638528859811704183484516925440.0f     // Maximum value of a float, from bit pattern 01111111011111111111111111111111

typedef Vector3 Position;

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
	
	// game loop
	while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
	{
		UpdateCamera(&camera, CAMERA_ORBITAL);
		
		// Draw
        //----------------------------------------------------------------------------------
		BeginDrawing();

			// Setup the back buffer for drawing (clear color and depth buffers)
			ClearBackground(BLACK);

			BeginMode3D(camera);

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
