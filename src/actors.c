#include "actors.h"
#include "headers.h"
#include "main.h"
#include "models.h"

Vector2 PickPerpendicular(Vector2 myDir, Vector2 wallDir);

Position MoveVector(Position position, Vector3 movement, float hitRadius, RayCollision * hit) {
    Vector3 moveNormal = Vector3Normalize(movement);
    Ray ray = { position, moveNormal };
    float moveDist = Vector3Length(movement);
    RayCollision c = RayToModels(ray);

    if(c.hit && c.distance - moveDist <= hitRadius) {
                
        // how much are we allowed to move?
        float realDist = c.distance - hitRadius;

        if(hit != NULL) {
            c.distance = realDist;
            *hit = c;
        }

        Vector3 realMove = Vector3Scale(moveNormal, realDist);
        return Vector3Add(position, realMove);
    }
    else {
        if(hit != NULL) {
            c.hit = false;
            *hit = c;
        }
        return Vector3Add(position, movement);
    }
}

Position MoveAndSlide(Position position, Vector3 movement, float hitRadius) {
    RayCollision c;
    float moveDist = Vector3Length(movement);
    Position newPos = MoveVector(position, movement, hitRadius, &c);
    
    // how much did we fail to move?
    float remaining = moveDist - c.distance;
    // try to slide
    if(c.hit && remaining > 0) {
        
        Vector2 slide2d = PickPerpendicular( (Vector2){ movement.x, movement.y }, 
            (Vector2){ c.normal.x, c.normal.y });
        
        Vector3 slide = Vector3Scale((Vector3){ slide2d.x, slide2d.y, 0.0f }, remaining);
        return MoveVector(newPos, slide, hitRadius, NULL);
    }
    else {
        return newPos;
    }
}

// not in header, since this likely will only be used in movement code
Vector2 PickPerpendicular(Vector2 myDir, Vector2 wallDir) {
    myDir = Vector2Normalize(myDir);
    wallDir = Vector2Normalize(wallDir);

    Vector2 perp1 = (Vector2){ -wallDir.y, wallDir.x };
    Vector2 perp2 = (Vector2){ wallDir.y, -wallDir.x };

    float dist1 = Vector2Distance(myDir, perp1);
    float dist2 = Vector2Distance(myDir, perp2);

    if(dist1 < dist2)
        return perp1;
    else
        return perp2;
}