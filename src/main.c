#include "headers.h"
#include "main.h"
#include "models.h"
#include "actors.h"

float GetRandomFloat(int min, int max, int precision) {
    float f = GetRandomValue(min*precision, max*precision);
    return f / precision;
}

// global
Camera camera = { 0 };
Vector3 up = { 0.0f, 0.0f, 1.0f };
Vector3 down = { 0.0f, 0.0f, -1.0f };
Vector3 north = { 0.0f, 1.0f, 0.0f };
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

    return (int)(cd2->dist*10000.0f - cd1->dist*10000.0f);
}

ecs_entity_t Billboards[SPRITE_COUNT];

DECLARE_PLIST(Image);
DECLARE_PLIST(Texture2D);

int main () {

	world = ecs_init();

	ECS_COMPONENT(world, Vector3);
	ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Billboard);
    ECS_COMPONENT(world, CamDistance);
    ECS_COMPONENT(world, ModelTransform);
    ECS_COMPONENT(world, Actor);

    ECS_MAP_COMPONENTS();
    ECS_MAP_QUERIES();

    ECS_SYSTEM(world, SetCamDistance, EcsOnUpdate, CamDistance, Position);


	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(1280, 800, "flecs Test");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Define the camera to look into our 3d world
    camera.position = (Vector3){ 0.0f, -12.0f, 8.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.0, 0.0f };      // Camera looking at point
    camera.up = up;          // Camera up vector (rotation towards target)
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

    Image img_sky = GenImageChecked(256, 256, 16, 16, BLACK, BLUE);
    Texture2D tex_sky = LoadTextureFromImage(img_sky);

    LIST_ADD(images, &fullimage);
    LIST_ADD(textures, &fulltex);
    LIST_ADD(images, &img_plain);
    LIST_ADD(textures, &tex_plain);
    LIST_ADD(images, &img_brick);
    LIST_ADD(textures, &tex_brick);
    LIST_ADD(images, &img_sky);
    LIST_ADD(textures, &tex_sky);

    // models

    LIST_(Model) model_res_list = NEWLIST(Model);

    // plain
    printf("LOAD PLAIN\n");
	Model model_plain = LoadModelFromMesh(GenMeshPlane2(16, 16, 16, 16));
	model_plain.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex_plain;

    MapModelCollection mapc_plain = MakeMapModelCollection(model_plain, &model_res_list);

    // block
    printf("LOAD BLOCK\n");
    Model model_block = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 3.0f));
	model_block.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex_brick;
    //Model model_block = LoadModel("Small pillar.glb");

    MapModelCollection mapc_block = MakeMapModelCollection(model_block, &model_res_list);

    // skybox
    printf("LOAD SKYBOX\n");
    Model model_skybox = LoadModelFromMesh(GenMeshInvertedCube(16, 16, 16));
	model_skybox.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex_sky;

    MapModelCollection mapc_skybox = MakeMapModelCollection(model_skybox, &model_res_list);

    // model entities
    // plain
    ecs_entity_t map_entity = ecs_new(world);
    ecs_set_maps(map_entity, mapc_plain);
    ecs_set(world, map_entity, ModelTransform, { .scale = unit_vector });

    // create blocks
    for(int i = 0; i < BLOCK_COUNT; i ++) {
        ecs_entity_t block_entity = ecs_new(world);

        Vector3 position = {
            GetRandomFloat(-8, 8, 1000),
            GetRandomFloat(-8, 8, 1000),
            GetRandomFloat(0, 1, 1000)
        };

        Vector3 rotationAxis = {
            GetRandomFloat(-1, 1, 1000),
            GetRandomFloat(-1, 1, 1000),
            GetRandomFloat(-1, 1, 1000)
        };
        
        float rotationAngle = GetRandomFloat(-30, 30, 1000);

        ecs_set_maps(block_entity, mapc_block);
        ecs_set(world, block_entity, ModelTransform, { .position = position, .scale = unit_vector,
            .rotationAxis = rotationAxis, .rotationAngle = rotationAngle });
    }

    // skybox
    ecs_entity_t skybox_entity = ecs_new(world);
    ecs_set_maps(skybox_entity, mapc_skybox);
    ecs_set(world, skybox_entity, ModelTransform, { .scale = unit_vector });

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
            (Vector2){ 0.5, ACTOR_SIZE_VECTORS[ACTOR_SIZE_SMALL].z},                         // origin
            WHITE,                                          // tint

        });
    }

    // create billboard guys
    for(int i = 0; i < ACTOR_COUNT; i ++) {
        int bb = GetRandomValue(SPRITE_RED, SPRITE_PURPLE);
        ecs_entity_t inst = ecs_new_w_pair(world, EcsIsA, Billboards[bb]);
        ecs_set(world, inst, CamDistance, { 0 });
        ecs_set(world, inst, Actor, { .size = ACTOR_SIZE_SMALL });

        float x = GetRandomFloat(-8, 8, 1000);
        float y = GetRandomFloat(-8, 8, 1000);
        float z = GetElevation(x, y, 8.0f, ACTOR_SIZE_SMALL) + ACTOR_HIT_MARGIN;
        if(z == FLT_MAX)
            z = 0.0f;

        ecs_set(world, inst, Position, { x, y, z });
    }
	
	//DisableCursor();

    ecs_query_t * q_billboards = ecs_query(world, {
        .terms = {
            { ecs_id(Billboard) }, { ecs_id(Position) }, { ecs_id(CamDistance) }
        },
        .order_by = ecs_id(CamDistance),
        .order_by_callback = (ecs_order_by_action_t)CompareCamDistance,
    });

    ecs_query_t * q_actors = ecs_query(world, {
        .terms = {
            { ecs_id(Actor) }, { ecs_id(Position) }
        }
    });

	// game loop
	while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
	{
        float dt = GetFrameTime() * 60.0;
        Timer += dt;
        ecs_progress(world, dt);

        if(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            UpdateCamera(&camera, CAMERA_FREE);
        
        float camAngle = atan2f(camera.target.y - camera.position.y, camera.target.x - camera.position.x);
        camAngle += 90*RAD2DEG;

        Vector2 keymove = {0};
        if(IsKeyDown(KEY_I))
            keymove.y += 1.0f;
        if(IsKeyDown(KEY_K))
            keymove.y -= 1.0;
        if(IsKeyDown(KEY_J))
            keymove.x -= 1.0f;
        if(IsKeyDown(KEY_L))
            keymove.x += 1.0f;
        
        keymove = Vector2Scale(keymove, 0.06f);
        keymove = Vector2Rotate(keymove, camAngle);

        Ray mouseRay = GetScreenToWorldRay(GetMousePosition(), camera);
        RayCollision mouseHit = RayToModels(mouseRay, ACTOR_SIZE_POINT, FLT_MAX);
		
		// Draw
        //----------------------------------------------------------------------------------
		BeginDrawing();

			// Setup the back buffer for drawing (clear color and depth buffers)
			ClearBackground(BLACK);

			BeginMode3D(camera);

                // draw models
                ecs_iter_t it = ecs_query_iter(world, q_MapModel);
                while(ecs_query_next(&it)) {
                    MapModel *models = ecs_field(&it, MapModel, 0);
                    ModelTransform *transforms = ecs_field(&it, ModelTransform, 1);
                    
                    // inner loop
                    for(int i = 0; i < it.count; i ++) {
                        Model model = models[i];
                        ModelTransform transform = transforms[i];

                        DrawModelEx(model, transform.position, transform.rotationAxis,
                            transform.rotationAngle, transform.scale, WHITE);
                    }
                }

                // draw model wires
#if DRAWWIRES || DRAWBBOX
                it = ecs_query_iter(world, q_MapModelEx[DRAWEXMESHSIZE]);
                while(ecs_query_next(&it)) {
                    MapModel *models = ecs_field(&it, MapModel, 0);
                    ModelTransform *transforms = ecs_field(&it, ModelTransform, 1);
                    MapBoxes *boxes = ecs_field(&it, MapBoxes, 2);
                    
                    // inner loop
                    for(int i = 0; i < it.count; i ++) {
                        ModelTransform transform = transforms[i];
                        BoundingBox box = boxes[i].modelBox;
#if DRAWWIRES
                        Color colors[] = {RED, WHITE, GREEN };
                        Model model = models[i];
                        DrawModelWiresEx(model, transform.position, transform.rotationAxis,
                            transform.rotationAngle, transform.scale, colors[ i % 3]);
#endif
#if DRAWBBOX
                        Matrix matTransform = MatrixFromTransform(transform);
                        // Check ray collision against bounding box first, before trying the full ray-mesh test
                        box = TransformBoundingBox(box, matTransform);
                        DrawBoundingBox(box, WHITE);
#endif
                    }
                }
#endif 

                if(mouseHit.hit) {
                    DrawCube(mouseHit.point, 0.3f, 0.3f, 0.3f, WHITE);
                    DrawCubeWires(mouseHit.point, 0.3f, 0.3f, 0.3f, RED);
                    DrawLine3D(mouseHit.point, Vector3Add(mouseHit.point, mouseHit.normal), RED);
                    DrawLine3D(mouseHit.point, Vector3Add(mouseHit.point, Vector2InPlane(V3toV2(north), mouseHit.normal)), GREEN);
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

        // actor movement
        it = ecs_query_iter(world, q_actors);

        while(ecs_query_next(&it)) {
            Actor *a = ecs_field(&it, Actor, 0);
            Position *p = ecs_field(&it, Position, 1);

            // inner loop
            for (int i = 0; i < it.count; i ++) {
                ActorPhysics( &a[i], &p[i], keymove );
            }
        }
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
        UnloadModel(LIST_GET(model_res_list, i));
    }

    // destroy the window and cleanup the OpenGL context
	CloseWindow();

	ecs_fini(world);

	return 0;
}