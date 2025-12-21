#include "actors.h"
#include "headers.h"
#include "main.h"
#include "models.h"

Vector2 PickPerpendicular(Vector2 myDir, Vector2 wallDir);

Position MoveActor(Position position, Vector2 movement, float hitRadius, RayCollision * hit) {
    
    Vector2 moveNormal = Vector2Normalize(movement);
    float moveDist = Vector2Length(movement);
    
    // center ray
    Ray ray_c = { position, V2toV3(moveNormal, 0) };

    Vector2 sideOffset = Vector2Scale(PERPL(moveNormal), hitRadius);
    Ray ray_l = { Vector3Add(position, V2toV3(sideOffset, 0)), V2toV3(moveNormal, 0)};
    Ray ray_r = { Vector3Subtract(position, V2toV3(sideOffset, 0)), V2toV3(moveNormal, 0)};

    RayCollision c_c = RayToModels(ray_c);

    RayCollision c;

    if(c.hit && c.distance - moveDist <= hitRadius) {
                
        // how much are we allowed to move?
        float realDist = c.distance - hitRadius;

        if(hit != NULL) {
            c.distance = realDist;
            *hit = c;
        }

        Vector3 realMove = Vector3Scale(V2toV3(moveNormal, 0), realDist);
        return Vector3Add(position, realMove);
    }
    else {
        if(hit != NULL) {
            c.hit = false;
            *hit = c;
        }
        return Vector3Add(position, V2toV3(movement, 0));
    }
}

Position MoveAndSlide(Position position, Vector2 movement, float hitRadius) {
    RayCollision c;
    float moveDist = Vector2Length(movement);
    Position newPos = MoveActor(position, movement, hitRadius, &c);
    
    // how much did we fail to move?
    float remaining = moveDist - c.distance;
    // try to slide
    if(c.hit && remaining > 0) {

        // TODO: use the dot product so we don't move full distance
        float slideDist = remaining;
        
        Vector2 slide = Vector2Scale(PickPerpendicular( movement, V3toV2(c.normal)), slideDist );
        
        return MoveActor(newPos, slide, hitRadius, NULL);
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