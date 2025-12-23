#include "actors.h"
#include "headers.h"
#include "main.h"
#include "models.h"

Vector3 ACTOR_SIZE_VECTORS[ACTOR_SIZE_COUNT] = {
    (Vector3){ 0.0f, 0.0f, 0.0f }, // point
    (Vector3){ ACTOR_SMALL_R, ACTOR_SMALL_R, ACTOR_SMALL_Ho2 }
};

Vector3 gravity = { 0.0f, 0.0f, -9.8f / 60.0f };

Vector2 PickPerpendicular(Vector2 myDir, Vector2 wallDir);

void ActorPhysics(Actor * actor, Position * position, Vector2 movement) {
    // first move
    *position = MoveAndSlideActor(*actor, *position, movement);
    // then apply gravity
    actor->velocity = Vector3Add(actor->velocity, gravity);

    // then check ground
    Vector3 hitcore = *position;
    hitcore.z += 0.1;
    RayCollision groundhit = RayToModels((Ray) { hitcore, down }, actor->size);
    groundhit.distance -= 0.1;

    float velDist = Vector3Length(actor->velocity);

    // snap up to terrain
    if(groundhit.hit && groundhit.distance <= 0.1) {
        position->z -= groundhit.distance;
        if(actor->velocity.z < 0) {
            actor->velocity = (Vector3){ 0 };
            //printf("STOP!\t%2.2f\t%p\n", groundhit.distance, actor);
        }

        if(IsKeyPressed(KEY_SPACE)) {
            actor->velocity = Vector3Scale(groundhit.normal, 0.75f);
            //printf("JUMP!\n");
        }
    }
    
    if(velDist > 0) {
        //printf("GO!\t%2.2f\t%p\n", groundhit.distance, actor);

        RayCollision velHit = RayToModels((Ray) { *position, Vector3Normalize(actor->velocity) }, actor->size );

        if(velHit.hit && velHit.distance < velDist) {
            //printf("OOF!\t%2.2f\t%p\n", velHit.distance, actor);
            if(velHit.distance > 0) {
                *position = Vector3Add(*position, Vector3Scale(Vector3Normalize(actor->velocity), velHit.distance));
            }
        }
        else {
            //printf("YAY!\t%2.2f\t%p\n", velHit.distance, actor);
            *position = Vector3Add(*position, actor->velocity);
        }
    }
}

Position MoveActor(Actor actor, Position position, Vector2 movement, RayCollision * hit) {

    Vector2 moveNormal = Vector2Normalize(movement);
    float moveDist = Vector2Length(movement);
    if(moveDist <= 0) {
        *hit = (RayCollision){ 0 };
        return position;
    }

    Vector3 move3D = V2toV3(moveNormal, 0);

    Vector3 hitcore = position;
    hitcore.z += 0.1;

    RayCollision groundhit = RayToModels((Ray) { hitcore, down }, actor.size);

    if(groundhit.hit && groundhit.distance <= 0.1) {
        move3D = Vector2InPlane(moveNormal, groundhit.normal);
    }
    
    // center ray
    Ray ray = { position, move3D };

    RayCollision c = RayToModels(ray, actor.size);

    if(moveDist >= c.distance) {
        
        /*
        DrawRay(ray, RED);
        */

        if(hit != NULL) {
            *hit = c;
        }
        if(c.distance < 0) {
            return position;
        }

        Vector3 realMove = Vector3Scale(move3D, c.distance);
        return Vector3Add(position, realMove);
    }
    else {
        
        /*
        DrawRay(ray, WHITE);
        */

        if(hit != NULL) {
            c.hit = false;
            *hit = c;
        }
        return Vector3Add(position, Vector3Scale(move3D, moveDist));
    }
}

Position MoveAndSlideActor(Actor actor, Position position, Vector2 movement) {
    RayCollision c;
    float moveDist = Vector2Length(movement);
    Position newPos = MoveActor(actor, position, movement, &c);
    
    // how much did we fail to move?
    float remaining = moveDist - (c.distance >= 0.0f ? c.distance : 0.0f);
    // try to slide
    if(c.hit && remaining > 0) {

        Vector2 perp = PickPerpendicular( movement, V3toV2(c.normal));
        // USE SCALAR PROJECTION
        float slideDist = remaining * cos(Vector2Angle(movement, perp));
        
        Vector2 slide = Vector2Scale(perp, slideDist );
        
        return MoveActor(actor, newPos, slide, NULL);
    }
    else {
        return newPos;
    }
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

Vector3 Vector2InPlane(Vector2 vec, Vector3 normal) {
    float z = -(normal.x * (vec.x) + normal.y * (vec.y))/normal.z;
    return Vector3Normalize((Vector3){ vec.x, vec.y, z });
}