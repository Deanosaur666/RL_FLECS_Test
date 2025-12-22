#include "actors.h"
#include "headers.h"
#include "main.h"
#include "models.h"

Vector3 gravity = { 0.0f, 0.0f, -9.8f / 60.0f };

Vector2 PickPerpendicular(Vector2 myDir, Vector2 wallDir);

void ActorPhysics(Actor * actor, Position * position, Vector2 movement) {
    // first move
    *position = MoveAndSlideActor(*actor, *position, movement);
    // then apply gravity
    actor->velocity = Vector3Add(actor->velocity, gravity);

    // then check ground
    Vector3 hitpos = Vector3Add(*position, Vector3Scale(up, actor->hitHeight));
    RayCollision groundhit = RayToModels((Ray) { hitpos, down });
    groundhit.distance -= actor->hitHeight;

    // snap up to terrain
    if(groundhit.hit && groundhit.distance <= 0) {
        position->z -= groundhit.distance;
        if(actor->velocity.z < 0) {
            actor->velocity = (Vector3){ 0 };
            printf("STOP!\t%2.2f\t%p\n", groundhit.distance, actor);
        }
    }

    float velDist = Vector3Length(actor->velocity);
    if(velDist > 0) {
        printf("GO!\t%2.2f\t%p\n", velDist, actor);

        RayCollision velHit = RayToModels((Ray) { hitpos, actor->velocity } );

        velHit.distance -= actor->hitHeight;

        if(velHit.hit && velHit.distance < velDist) {
            if(velHit.distance > 0) {
                *position = Vector3Add(*position, Vector3Scale(Vector3Normalize(actor->velocity), velHit.distance));
            }
        }
        else {
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

    float hitRadius = actor.radius;
    Vector3 hitpos = Vector3Add(position, Vector3Scale(up, actor.hitHeight));
    Vector3 move3D = V2toV3(moveNormal, 0);

    RayCollision groundhit = RayToModels((Ray) {hitpos, down });
    groundhit.distance -= actor.hitHeight;

    if(groundhit.hit && groundhit.distance <= 0.1) {
        move3D = Vector2InPlane(moveNormal, groundhit.normal);
    }
    
    // center ray
    Ray ray_c = { hitpos, move3D };

    Vector2 sideOffset = Vector2Scale(PERPL(moveNormal), hitRadius/2);
    Ray ray_l = { Vector3Add(hitpos, V2toV3(sideOffset, 0)), move3D};
    Ray ray_r = { Vector3Subtract(hitpos, V2toV3(sideOffset, 0)), move3D};

    RayCollision c_c = RayToModels(ray_c);
    c_c.distance -= hitRadius;
    RayCollision c_l = RayToModels(ray_l);
    c_l.distance -= hitRadius;
    RayCollision c_r = RayToModels(ray_r);
    c_r.distance -= hitRadius;

    RayCollision c = c_c;
    if(c_l.distance < c.distance) {
        c = c_l;
    }
    if(c_r.distance < c.distance) {
        c = c_r;
    }

    /*
    printf("L: %3.2f C: %3.2f R: %3.2f P: %3.2f\n", c_l.distance < 100.0f ? c_l.distance : 999.99f, 
        c_c.distance < 100.0f ? c_c.distance : 999.99f,
        c_r.distance < 100.0f ? c_r.distance : 999.99f,
        c.distance < 100.0f ? c.distance : 999.99f);
    */

    if(moveDist >= c.distance) {
        
        /*
        DrawRay(ray_c, RED);
        DrawRay(ray_l, RED);
        DrawRay(ray_r, RED);
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
        DrawRay(ray_c, WHITE);
        DrawRay(ray_l, WHITE);
        DrawRay(ray_r, WHITE);
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