#include "actors.h"
#include "headers.h"
#include "main.h"
#include "models.h"

#define VECTOR_IS_NAN(vector) \
(isnan(vector.x) || isnan(vector.y) || isnan(vector.z))

#define VECTOR_PTR_IS_NAN(vector) \
(isnan(vector->x) || isnan(vector->y) || isnan(vector->z))

Vector3 ACTOR_SIZE_VECTORS[ACTOR_SIZE_COUNT] = {
    (Vector3){ 0.0f, 0.0f, 0.0f }, // point
    (Vector3){ ACTOR_SMALL_R, ACTOR_SMALL_R, ACTOR_SMALL_Ho2 }
};

Vector3 gravity = { 0.0f, 0.0f, -GRAVITY };

Vector2 PickPerpendicular(Vector2 myDir, Vector2 wallDir);

void ActorPhysics(Actor * actor, Position * position, Vector2 movement) {

    movement = Vector2Scale(movement, ACTOR_SPEED);
    Vector3 moveXY = V2toV3(movement, 0);

    // apply gravity
    actor->velocity = Vector3Add(actor->velocity, gravity);
    
    // then check the ground
    Vector3 hitcore = *position;
    hitcore.z += ACTOR_HIT_MARGIN;
    RayCollision groundhit = RayToModels((Ray) { hitcore, down }, actor->size, FLT_MAX);
    groundhit.distance -= ACTOR_HIT_MARGIN;

    Vector3 groundNormal = up;
    
    Vector3 moveZ = { 0.0f, 0.0f, actor->velocity.z};
    if(moveZ.z < 0) {
        groundhit.distance -= MoveActorRayCollision(actor, position, moveZ, groundhit);
    }
    else if(moveZ.z > 0) {
        MoveActor(actor, position, hitcore, moveZ, NULL);
    }

    // on ground
    if(groundhit.hit && groundhit.distance <= ACTOR_HIT_MARGIN) {
        
        float groundAngle = Vector3Angle(up, groundhit.normal) * RAD2DEG;

        if(groundAngle <= 60.0f) {
            groundNormal = groundhit.normal;
            
            // tilt vector
            moveXY = GetTiltVector(movement, groundNormal);
        }

        // snap to terrain
        position->z -= groundhit.distance;
        groundhit.distance = 0.0f;
        if(actor->velocity.z < 0) {
            actor->velocity = (Vector3){ 0.0f, 0.0f, 0.0f };
        }

        // jump
        if(IsKeyPressed(KEY_SPACE)) {
            actor->velocity = Vector3Add(actor->velocity, Vector3Scale(groundNormal, 0.3f));
            //actor->velocity = Vector3Add(actor->velocity, Vector3Scale(up, 0.2f));
        }
    }
    // not on ground
    else {
        
    }

    // add velocity
    moveXY.x += actor->velocity.x;
    moveXY.y += actor->velocity.y;

    float moveDist = Vector3Length(moveXY);

    // move and slide
    if(moveDist > 0.0f) {
        hitcore = *position;
        hitcore.z += ACTOR_HIT_MARGIN;

        RayCollision c = { 0 };
        float realMoveDist = MoveActor(actor, position, hitcore, moveXY, &c);
        
        float reminaingMove = Vector2Length(movement) - realMoveDist;
        // only slide if 2D movement is non-zero
        if(c.hit && reminaingMove > 0 && Vector2Length(movement) > 0) {
            hitcore = *position;
            hitcore.z += ACTOR_HIT_MARGIN;

            actor->velocity = (Vector3) { 0.0f, 0.0f, actor->velocity.z };

            Vector2 perp = PickPerpendicular(V3toV2(moveXY), V3toV2(c.normal));
            float slideDist = reminaingMove * cos(Vector2Angle(V3toV2(moveXY), perp));
            
            Vector2 slide = Vector2Scale(perp, slideDist);
            Vector3 slide3D = GetTiltVector(slide, groundNormal);

            MoveActor(actor, position, hitcore, slide3D, NULL);
        }
        
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
    RayCollision c = RayToModels(ray, actor->size, moveDist + ACTOR_HIT_MARGIN*2);
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