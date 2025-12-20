#include "headers.h"
#include "main.h"
#include "models.h"
#include "list.h"
#include "actors.h"

float GetRandomFloat(int min, int max, int precision) {
    float f = GetRandomValue(min*precision, max*precision);
    return f / precision;
}

// global
Camera camera = { 0 };
Vector3 up = { 0.0f, 1.0f, 0.0f };
Vector3 down = { 0.0f, -1.0f, 0.0f };
Vector3 unit_vector = { 1.0f, 1.0f, 1.0f };
ecs_world_t * world;

float Timer = 0;

typedef struct Billboard {
    Texture2D * tex;
    Rectangle source;
    Vector2 size;
    Vector2 origin;
    Color tint;
} Billboard;

typedef struct CamDistance {
    float dist;
} CamDistance;

void SetCamDistance(ecs_iter_t * it) {
    CamDistance * cd = ecs_field(it, CamDistance, 0);
    Position * pos = ecs_field(it, Position, 1);

    // iterate matched entites
    for(int i = 0; i < it->count; i ++) {
        cd[i].dist = Vector3Distance(camera.position, pos[i]);
    }
}

int CompareCamDistance(ecs_entity_t e1, const void * v1, ecs_entity_t e2, const void *v2) {
    const CamDistance * cd1 = v1;
    const CamDistance * cd2 = v2;

    return (int)(cd2->dist*1000.0f - cd1->dist*1000.0f);
}

ecs_entity_t Billboards[SPRITE_COUNT];

float GetElevation(float x, float y, float z);

DECLARE_PLIST(Image);
DECLARE_PLIST(Texture2D);
DECLARE_PLIST(Model);

int main () {

	world = ecs_init();

	ECS_COMPONENT(world, Vector3);
	ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Billboard);
    ECS_COMPONENT(world, CamDistance);
    ECS_COMPONENT(world, MapModel);
    ECS_COMPONENT(world, ModelTransform);

    ECS_SYSTEM(world, SetCamDistance, EcsOnUpdate, CamDistance, Position);

    q_models = ecs_query(world, {
        .terms = {
            { ecs_id(MapModel) }, { ecs_id(ModelTransform) }
        },
    });


	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(1280, 800, "flecs Test");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Define the camera to look into our 3d world
    camera.position = (Vector3){ 0.0f, 1.8f, 0.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 1.8f, -1.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

	SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    // textures
    // lists for removing when done
    PLIST_(Image) images = NEWPLIST(Image);
    PLIST_(Texture2D) textures = NEWPLIST(Texture2D);

	Image fullimage = LoadImage("sprites.png");
    Texture2D fulltex = LoadTextureFromImage(fullimage);
	Image img_plain = ImageFromImage(fullimage, (Rectangle){ 16*4, 16*2, 16, 16 });
	Texture2D tex_plain = LoadTextureFromImage(img_plain);
    Image img_brick = ImageFromImage(fullimage, (Rectangle){ 16*2, 16*2, 16, 16 });
	Texture2D tex_brick = LoadTextureFromImage(img_brick);

    LIST_ADD(images, &fullimage);
    LIST_ADD(textures, &fulltex);
    LIST_ADD(images, &img_plain);
    LIST_ADD(textures, &tex_plain);
    LIST_ADD(images, &img_brick);
    LIST_ADD(textures, &tex_brick);

    // models

    PLIST_(Model) model_res_list = NEWPLIST(Model);

    // plain
	Model model_plain = LoadModelFromMesh(GenMeshPlane2(16, 16, 16, 16));
	model_plain.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex_plain;

    LIST_ADD(model_res_list, &model_plain);

    // block
    Model model_block = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
	model_block.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex_brick;

    LIST_ADD(model_res_list, &model_block);

    // model entities
    ecs_entity_t map_entity = ecs_new(world);
    ecs_set(world, map_entity, MapModel, { model_plain });
    ecs_set(world, map_entity, ModelTransform, { .scale = unit_vector });

    // create blocks
    for(int i = 0; i < 32; i ++) {
        ecs_entity_t block_entity = ecs_new(world);

        Vector3 position = {
            GetRandomFloat(-8, 8, 1000),
            GetRandomFloat(0, 1, 1000),
            GetRandomFloat(-8, 8, 1000)
        };

        Vector3 scale = {
            1.0f, GetRandomFloat(1, 3, 1000), 1.0f
        };

        Vector3 rotationAxis = {
            GetRandomFloat(-1, 1, 1000),
            GetRandomFloat(-1, 1, 1000),
            GetRandomFloat(-1, 1, 1000)
        };
        
        float rotationAngle = GetRandomFloat(-30, 30, 1000);


        ecs_set(world, block_entity, MapModel, { model_block });
        ecs_set(world, block_entity, ModelTransform, { .position = position, .scale = scale,
            .rotationAxis = rotationAxis, .rotationAngle = rotationAngle });
    }

    // sprite billboard prefabs
    for(int i = SPRITE_RED; i <= SPRITE_PURPLE; i ++) {
        float tilew = ((float)fulltex.width)/8.0f;
        float tileh = ((float)fulltex.height)/8.0f;
        Billboards[i] = ecs_entity(world, {
            .add = ecs_ids( EcsPrefab )
        });
        ecs_set(world, Billboards[i], Billboard, {
            &fulltex,                                       // tex
            (Rectangle){ i * tilew, 0.0f, tilew, tileh},    // source
            (Vector2){ 1.0f, 1.0f },                        // size
            (Vector2){ 0.5, 0.0f },                         // origin
            WHITE,                                          // tint

        });
    }

    // create billboard guys
    for(int i = 0; i < 64; i ++) {
        int bb = GetRandomValue(SPRITE_RED, SPRITE_PURPLE);
        ecs_entity_t inst = ecs_new_w_pair(world, EcsIsA, Billboards[bb]);
        ecs_set(world, inst, CamDistance, { 0 });

        float x = GetRandomFloat(-8, 8, 1000);
        float z = GetRandomFloat(-8, 8, 1000);
        float y = GetElevation(x, 16.0f, z);
        if(y == FLT_MAX)
            y = 0.0f;

        ecs_set(world, inst, Position, { x, y, z });
    }
	
	DisableCursor();

    ecs_query_t * q_billboards = ecs_query(world, {
        .terms = {
            { ecs_id(Billboard) }, { ecs_id(Position) }, { ecs_id(CamDistance) }
        },
        .order_by = ecs_id(CamDistance),
        .order_by_callback = (ecs_order_by_action_t)CompareCamDistance,
    });

	// game loop
	while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
	{
        float dt = GetFrameTime() * 60.0;
        Timer += dt;
		UpdateCamera(&camera, CAMERA_FREE);
        ecs_progress(world, dt);
		
		// Draw
        //----------------------------------------------------------------------------------
		BeginDrawing();

			// Setup the back buffer for drawing (clear color and depth buffers)
			ClearBackground(BLACK);

			BeginMode3D(camera);

                // draw models
                ecs_iter_t it = ecs_query_iter(world, q_models);
                while(ecs_query_next(&it)) {
                    MapModel *models = ecs_field(&it, MapModel, 0);
                    ModelTransform *transforms = ecs_field(&it, ModelTransform, 1);
                    
                    // inner loop
                    for(int i = 0; i < it.count; i ++) {
                        Model model = models[i].model;
                        ModelTransform transform = transforms[i];

                        DrawModelEx(model, transform.position, transform.rotationAxis,
                            transform.rotationAngle, transform.scale, WHITE);
                    }
                }
				
                // draw billboards
                it = ecs_query_iter(world, q_billboards);

                while(ecs_query_next(&it)) {
                    Billboard *b = ecs_field(&it, Billboard, 0);
                    Position *p = ecs_field(&it, Position, 1);

                    // inner loop
                    for (int i = 0; i < it.count; i ++) {
                        DrawBillboardPro(camera, *b[i].tex, b[i].source, p[i], up, b[i].size, b[i].origin, 0.0f, b[i].tint);
                    }
                }

			EndMode3D();
		
			DrawFPS(10, 10);
		
		EndDrawing();
        //----------------------------------------------------------------------------------
	}

	// cleanup

    // TODO: unload images, textures, and models
    for(int i = 0; i < textures.size; i ++) {
        UnloadTexture(*LIST_GET(textures, i));
    }
    for(int i = 0; i < images.size; i ++) {
        UnloadImage(*LIST_GET(images, i));
    }
    for(int i = 0; i < model_res_list.size; i ++) {
        printf("%d\n", i);
        UnloadModel(*LIST_GET(model_res_list, i));
    }

    // destroy the window and cleanup the OpenGL context
	CloseWindow();

	ecs_fini(world);

	return 0;
}