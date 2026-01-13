#include "actors.h"
#include "headers.h"
#include "main.h"
#include "models.h"
#include "collision.h"

Vector3 gravity = { 0.0f, 0.0f, -GRAVITY };

Vector2 PickPerpendicular(Vector2 myDir, Vector2 wallDir);

// Only apply friction when on ground
void ActorFriction(Actor * actor) {
    float speed = Vector3Length(actor->velocity);
    if(speed < ACTOR_MIN_SPEED) {
        actor->velocity.x = 0;
        actor->velocity.y = 0;
        actor->velocity.z = 0;
        return;
    }

    float newSpeed = speed - ACTOR_FRICTION;
    if(newSpeed < 0)
        newSpeed = 0;
    
    actor->velocity.x *= newSpeed/speed;
    actor->velocity.y *= newSpeed/speed;
    actor->velocity.z *= newSpeed/speed;
}

void ActorAccelerate(Actor * actor, Vector3 movedir, bool grounded) {
    assert(!VECTOR3_IS_NAN(actor->velocity));
    Vector3 velocityXY = V2toV3(actor->velocity, 0);
    float currentSpeed = Vector3DotProduct(velocityXY, movedir);
    float maxSpeed = ACTOR_MAX_SPEED;
    if(!grounded)
        maxSpeed = ACTOR_AIR_MAX_SPEED;
    
    float addspeed = maxSpeed - currentSpeed;
    
    if(addspeed <= 0)
        return;
    float accel = ACTOR_ACCEL > addspeed ? addspeed : ACTOR_ACCEL;

    actor->velocity = Vector3Add(actor->velocity, Vector3Scale(movedir, accel));
    assert(!VECTOR3_IS_NAN(actor->velocity));
}

void ActorPhysics(Actor * actor, Position * position, Vector2 movedir) {

    // CHECK GROUNDED
    // TEST A POINT BELOW ACTOR
    Collision groundCollision = { 0 };
    groundCollision.hit = false;
    groundCollision.depth = 0.0f;
    groundCollision.direction = up;

    ActorTestGround(actor, position, &groundCollision);
    if(groundCollision.hit) {
        actor->grounded = ACTOR_GROUND_TIME;
        if(fabsf(groundCollision.depth) > ACTOR_GROUND_TEST_DIST)
            position->z -= (groundCollision.depth - ACTOR_GROUND_TEST_DIST);
        else if(fabsf(groundCollision.depth) < ACTOR_GROUND_TEST_DIST/2)
            position->z -= ACTOR_GROUND_TEST_DIST/2;
    }
    else {
        // apply gravity
        actor->velocity = Vector3Add(actor->velocity, gravity);
    }

    // grounded
    if(actor->grounded > 0) {
        //Vector3 accel = GetTiltVector(movedir, actor->groundNormal);
        Vector3 accel = V2toV3(movedir, 0);
        ActorFriction(actor);
        ActorAccelerate(actor, accel, true);

        // if on ground, cap downward velocity to prevent too much sliding
        if(actor->velocity.z < -ACTOR_MAX_SPEED) {
            actor->velocity.z = -ACTOR_MAX_SPEED;
        }

        // remove component against ground
        if(actor->grounded == ACTOR_GROUND_TIME) {
            actor->velocity = ClipVector(actor->velocity, actor->groundNormal);
        }

        // jump
        if(IsKeyPressed(KEY_SPACE)) {
            actor->velocity = Vector3Add(actor->velocity, Vector3Scale(up, 0.5f));
            actor->grounded = 0;
        }
    }
    else {
        ActorAccelerate(actor, V2toV3(movedir, 0), false);
    }

    actor->grounded --;

    MoveActorBox(actor, position, actor->velocity, &groundCollision);
    //MoveActorBox(actor, position, actor->velocity, NULL);
    if(groundCollision.hit) {
        actor->groundNormal = groundCollision.direction;
    }
    //DrawRay((Ray){ *position, actor->groundNormal }, GREEN);
    //DrawRay((Ray){ *position, actor->velocity }, YELLOW);
    
}

// not in header, since this likely will only be used in movement code
Vector2 PickPerpendicular(Vector2 myDir, Vector2 wallDir) {
    myDir = Vector2Normalize(myDir);
    wallDir = Vector2Normalize(wallDir);

    Vector2 perpl = PERPL(wallDir);
    Vector2 perpr = PERPR(wallDir);

    float distl = Vector2Distance(myDir, perpl);
    float distr = Vector2Distance(myDir, perpr);

    if(distl < distr)
        return perpl;
    else
        return perpr;
}

Vector3 GetTiltVector(Vector2 dir2D, Vector3 normal) {
    float z = -(normal.x * (dir2D.x) + normal.y * (dir2D.y))/normal.z;
    return Vector3Scale(Vector3Normalize((Vector3){ dir2D.x, dir2D.y, z }), Vector2Length(dir2D));
}

float GetElevation(float x, float y, float z) {
    Ray ray = { {x, y, z}, down };
    RayCollision collision = RayToAnyCollider(ray, FLT_MAX);

    if(collision.hit) {
        return collision.point.z;
    }

    return FLT_MAX;
}

void ActorCollision(Actor * actor, Position * position, Collision c, Vector3 * move, Collision * groundCollision) {
    if(move != NULL) {
        Vector3 moveNormal = Vector3Normalize(*move);
        float moveDist = Vector3Length(*move);
        
        float eject = Vector3DotProduct(moveNormal, Vector3Negate(c.direction)) * c.depth;
        moveDist = moveDist - eject;
        if(moveDist < 0)
            moveDist = 0;
        *move = Vector3Scale(moveNormal, moveDist);
        if(Vector3Angle(*move, c.direction) > 90.0f*DEG2RAD)
            actor->velocity = ClipVector(actor->velocity, c.direction);
    }

    if(groundCollision != NULL) {
        float groundAngle = Vector3Angle(up, c.direction);
        if(groundAngle <= ACTOR_MAX_SLOPE) {
            actor->grounded = ACTOR_GROUND_TIME;
            groundCollision->hit = true;
            if(Vector3DotProduct(c.direction, up) < Vector3DotProduct(groundCollision->direction, up)) {
                groundCollision->direction = c.direction;
            }
            if(c.depth > groundCollision->depth) {
                groundCollision->depth = c.depth;
                groundCollision->point = c.point;
            }
        }
    }

    //actor->velocity = ClipVector(actor->velocity, c.direction);

    //DrawRay((Ray){ *position, c.direction }, RED);
}

float MoveActorBox(Actor * actor, Position * position, Vector3 move, Collision * groundCollision) {

    float moveDist = Vector3Length(move);
    Vector3 moveNormal = Vector3Normalize(move);

    // what's our box after moving the full distance?
    Position target = Vector3Add(*position, move);
    BoundingBox targetBox = BoundingBoxAdd(*actor->box, target);

    DrawBoundingBox(targetBox, RED);

    // boxes (non actor)
    ecs_iter_t it = ecs_query_iter(world, q_BoxColliderNotActor);
    while (ecs_query_next(&it))  {
        BoxCollider * colliders = ecs_field(&it, BoxCollider, 0);

        for(int i = 0; i < it.count; i ++) {
            BoxCollider box = colliders[i];

            Collision c = BoxBoxCollision(targetBox, box);
            if(c.hit) {
                ActorCollision(actor, position, c, &move, groundCollision);
            }
        }
    }

    // meshes
    it = ecs_query_iter(world, q_MeshCollider);
    while (ecs_query_next(&it))  {
        MeshCollider * colliders = ecs_field(&it, MeshCollider, 0);

        for(int i = 0; i < it.count; i ++) {
            MeshCollider collider = colliders[i];

            Collision c = BoxMeshCollision(targetBox, collider);

            if(c.hit) {
                ActorCollision(actor, position, c, &move, groundCollision);
            }
        }
    }
    *position = Vector3Add(*position, move);

    return moveDist;
}

void ActorTestGround(Actor * actor, Position * position, Collision * groundCollision) {

    // what's our box after moving the full distance?
    Position target = *position;
    target.z -= ACTOR_GROUND_TEST_DIST;

    //BoundingBox targetBox = BoundingBoxAdd(*actor->box, target);

    DrawCube(target, 0.05f, 0.05f, 0.05f, RED);

    // boxes (non actor)
    ecs_iter_t it = ecs_query_iter(world, q_BoxColliderNotActor);
    while (ecs_query_next(&it))  {
        BoxCollider * colliders = ecs_field(&it, BoxCollider, 0);

        for(int i = 0; i < it.count; i ++) {
            BoxCollider box = colliders[i];

            Collision c = PointBoxCollision(target, box);
            //Collision c = BoxBoxCollision(targetBox, box);

            if(c.hit) {
                ActorCollision(actor, position, c, NULL, groundCollision);
            }
        }
    }

    // meshes
    it = ecs_query_iter(world, q_MeshCollider);
    while (ecs_query_next(&it))  {
        MeshCollider * colliders = ecs_field(&it, MeshCollider, 0);

        for(int i = 0; i < it.count; i ++) {
            MeshCollider collider = colliders[i];

            Collision c = PointMeshCollision(target, collider);
            //Collision c = BoxMeshCollision(targetBox, collider);

            if(c.hit) {
                ActorCollision(actor, position, c, NULL, groundCollision);
            }
        }
    }

    groundCollision->depth -= ACTOR_GROUND_TEST_DIST;
}