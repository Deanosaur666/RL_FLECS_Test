#include "actors.h"
#include "headers.h"
#include "main.h"
#include "models.h"
#include "collision.h"

Vector3 gravity = { 0.0f, 0.0f, -GRAVITY };

Vector2 PickPerpendicular(Vector2 myDir, Vector2 wallDir);

// Only apply friction when on ground
void ActorFriction(Actor * actor) {
    float speed = Vector2Length(V3toV2(actor->velocity));
    if(speed < ACTOR_MIN_SPEED) {
        actor->velocity.x = 0;
        actor->velocity.y = 0;
        return;
    }

    float newSpeed = speed - ACTOR_FRICTION;
    if(newSpeed < 0)
        newSpeed = 0;
    
    actor->velocity.x *= newSpeed/speed;
    actor->velocity.y *= newSpeed/speed;
}

void ActorAccelerate(Actor * actor, Vector3 movedir, bool grounded) {
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
}

void ActorPhysics(Actor * actor, Position * position, Vector2 movedir) {

    Vector3 moveXY;

    // apply gravity
    actor->velocity = Vector3Add(actor->velocity, gravity);

    // grounded
    if(actor->grounded > 0) {
        Vector3 accel = GetTiltVector(V3toV2(movedir), actor->groundNormal);
        ActorFriction(actor);
        ActorAccelerate(actor, accel, true);

        // remove component against ground
        if(actor->grounded == ACTOR_GROUND_TIME) {
            float backoff = Vector3DotProduct(actor->velocity, actor->groundNormal) * (actor->grounded / ACTOR_GROUND_TIME);
            Vector3 eject = Vector3Scale(actor->groundNormal, backoff);
            actor->velocity = Vector3Subtract(actor->velocity, eject);
        }

        // jump
        if(IsKeyPressed(KEY_SPACE)) {
            //actor->velocity = Vector3Add(actor->velocity, Vector3Scale(groundNormal, 0.3f));
            actor->velocity = Vector3Add(actor->velocity, Vector3Scale(up, 0.5f));
        }
    }
    else {
        ActorAccelerate(actor, V2toV3(movedir, 0), false);
    }

    actor->grounded --;

    Collision groundCollision = { false };
    MoveActorBox(actor, position, actor->velocity, &groundCollision);
    if(groundCollision.hit) {
        actor->groundNormal = groundCollision.direction;
        if(groundCollision.depth > GRAVITY * 2)
            position->z += groundCollision.depth;
    }
    

}

float MoveActorRayCollision(Actor * actor, Position * position, Vector3 movement, RayCollision c) {
    
    float moveDist = Vector3Length(movement);
    Vector3 moveNormal = Vector3Normalize(movement);

    if(c.hit && c.distance <= moveDist + ACTOR_HIT_MARGIN) {

        float realMoveDist = c.distance - ACTOR_HIT_MARGIN;
        if(c.distance > ACTOR_HIT_MARGIN && realMoveDist > 0) {
            Vector3 realMove = Vector3Scale(moveNormal, realMoveDist);
            *position = Vector3Add(*position, realMove);

            return realMoveDist; // we moved a partial distance
        }
        else
            return 0.0f; // no movement
    }
    // movement vector is completely free
    else {
        *position = Vector3Add(*position, movement);

        return moveDist; // we moved the full distance
    }
}

float MoveActor(Actor * actor, Position * position, Vector3 hitcore, Vector3 movement, RayCollision * rc) {
    float moveDist = Vector3Length(movement);
    Vector3 moveNormal = Vector3Normalize(movement);
    Ray ray = { hitcore, moveNormal };
    RayCollision c = RayToAnyCollider(ray, moveDist + ACTOR_HIT_MARGIN*2);
    if(rc != NULL)
        *rc = c;
    
    return MoveActorRayCollision(actor, position, movement, c);
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

void ActorCollision(Actor * actor, Collision c, Vector3 * move, Collision * groundCollision) {
    Vector3 moveNormal = Vector3Normalize(*move);
    float moveDist = Vector3Length(*move);

    float eject = Vector3DotProduct(moveNormal, Vector3Negate(c.direction)) * c.depth;
    moveDist = moveDist - eject;
    *move = Vector3Scale(moveNormal, moveDist);

    float groundAngle = Vector3Angle(up, c.direction);
    if(groundAngle <= ACTOR_MAX_SLOPE) {
        actor->grounded = ACTOR_GROUND_TIME;
        groundCollision->hit = true;
        if(Vector3DotProduct(c.direction, up) < Vector3DotProduct(groundCollision->direction, up)) {
            groundCollision->direction = c.direction;
        }
        if(c.depth > groundCollision->depth) {
            groundCollision->depth = c.depth;
        }
    }

    DrawRay((Ray){ c.point, c.direction }, RED);
}

float MoveActorBox(Actor * actor, Position * position, Vector3 move, Collision * groundCollision) {
    
    groundCollision->hit = false;
    groundCollision->depth = 0.0f;
    groundCollision->direction = up;

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
                ActorCollision(actor, c, &move, groundCollision);
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
                ActorCollision(actor, c, &move, groundCollision);

            }
        }
    }

    *position = Vector3Add(*position, move);

    return moveDist;
}