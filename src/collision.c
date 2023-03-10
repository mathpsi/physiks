#include "collision.h"
#include "object.h"
#include "vector.h"

#include <glad/glad.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

int is_colliding(Object *object_1, Object *object_2) {
    for (int i = 0; sizeof(object_1->collision->collide)/sizeof(GLfloat); i++) {
        if (object_1->collision->collide[i] == object_2->id) {
	    return 1;
	} else {
	    return 0;
	}
    }
}

int CircularCollision(Object *object_1, Object *object_2) {
    if (object_1->shape->shape != CIRCLE || object_2->shape->shape != CIRCLE) {
        fprintf(stderr, "WARN_INCOMPATIBLE_COLLISION\n");      
    }
    
    /* Distance = sqrt((x1 - x2)^2 + (y1 - y2)^2) */
    GLfloat distance = sqrt(pow(object_1->position.x - object_2->position.x, 2) + pow(object_1->position.y - object_2->position.y, 2));
    
    if (object_1->shape->radius + object_2->shape->radius >= distance) {
	object_1->collision->collide_count++; object_2->collision->collide_count++;
	object_1->collision->collide = realloc(object_1->collision->collide, sizeof(GLuint*) * object_1->collision->collide_count);
	object_2->collision->collide = realloc(object_2->collision->collide, sizeof(GLuint*) * object_2->collision->collide_count);
	object_1->collision->collide[object_1->collision->collide_count - 1] = object_2->id;
	object_2->collision->collide[object_2->collision->collide_count - 1] = object_1->id;
        return 1;
    } else if (object_1->shape->radius + object_2->shape->radius <= distance) {
        if (object_1->collision->collide_count > 0 && object_2->collision->collide_count > 0) {
	    object_1->collision->collide_count--; object_2->collision->collide_count--;
	}
        return 0;
    } else {
        return -1; /* Error */
    }
}

int RectangularCollision(Object *object_1, Object *object_2) {
    if (object_1->shape->shape != RECTANGLE || object_2->shape->shape != RECTANGLE) {
        fprintf(stderr, "WARN_INCOMPATIBLE_COLLISION\n");      
    }

    if (
        object_1->position.x + object_1->shape->size.x >= object_2->position.x &&
        object_1->position.y + object_1->shape->size.y >= object_2->position.y &&
	object_2->position.x + object_2->shape->size.x >= object_1->position.x &&
	object_2->position.y + object_2->shape->size.y >= object_1->position.y
       ) {
        object_1->collision->collide_count++; object_2->collision->collide_count++;
	object_1->collision->collide = realloc(object_1->collision->collide, sizeof(GLuint*) * object_1->collision->collide_count);
	object_2->collision->collide = realloc(object_2->collision->collide, sizeof(GLuint*) * object_2->collision->collide_count);
	object_1->collision->collide[object_1->collision->collide_count - 1] = object_2->id;
	object_2->collision->collide[object_2->collision->collide_count - 1] = object_1->id;
        return 1;
    } else if (
	       !(object_1->position.x + object_1->shape->size.x >= object_2->position.x  &&
		 object_1->position.y + object_1->shape->size.y >= object_2->position.y &&
		 object_2->position.x + object_2->shape->size.x >= object_1->position.x  &&
		 object_2->position.y + object_2->shape->size.y >= object_1->position.y)
	       ) {
        if (object_1->collision->collide_count > 0 && object_2->collision->collide_count > 0) {
	    object_1->collision->collide_count--; object_2->collision->collide_count--;
	}
        return 0;
    } else {
        return -1;
    }
}
