#include "object.h"
#include "collision.h"
#include "shape.h"
#include "vector.h"
#include "physics.h"
#include "shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

GLfloat zoom_value = 2.0f;

Object *InitializeObject(Vector2 position, Shape_t shape, Vector2 size, GLfloat radius, Color color, Renderer *renderer) {
    Object *object = malloc(sizeof(Object));
    object->collision = malloc(sizeof(Collision));
    object->shape = malloc(sizeof(Shape));
    object->rigidbody = calloc(1, sizeof(Rigidbody));
    // object->rigidbody->force = malloc(sizeof(Force) * 10);
    object->shape->shape = shape;
    object->color = color;
    
    if (shape != RECTANGLE && shape != CIRCLE) {
        fprintf(stderr, "ERR_UNKOWN_SHAPE\n");
    }
    object->collision->collide = malloc(sizeof(GLuint*));
    renderer->objects = realloc(renderer->objects, sizeof(Object**) * (renderer->object_count + 1));
    object->position = position; object->shape->shape = shape;  object->id = renderer->object_count; object->shape->size = size; object->shape->radius = radius;
    renderer->objects[renderer->object_count] = object; renderer->object_count++; object->collision->collide_count = 0;
    return object;
}

Renderer *InitializeRenderer(GLuint program_id) {
    Renderer *renderer = malloc(sizeof(Renderer));
    renderer->objects = malloc(sizeof(Object**));
    GLuint zoom = glGetUniformLocation(program_id, "zoom");
    GLuint move = glGetUniformLocation(program_id, "move");
    GLuint model = glGetUniformLocation(program_id, "model_wh");
    GLuint color = glGetUniformLocation(program_id, "new_color");
    
    renderer->unit_circle = GenerateCircle(1.0f, CIRCLE_QUALITY); /* 32 quality */
    renderer->indices_circle = GenerateCircleIndices(CIRCLE_QUALITY);
    
    GLuint vao, vbo, ebo;
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    
    renderer->object_count = 0; renderer->vao = vao; renderer->move = move; renderer->model = model; renderer->color = color; renderer->ebo = ebo; renderer->program_id = program_id; renderer->vbo = vbo; renderer->zoom = zoom;
    
    return renderer;
}

void RenderObjects(Renderer *renderer) {
    /* Clear the screen */
    glClearColor(0.0f, 0.5f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(renderer->program_id);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    
    /* Draw objects */
    /* TODO: Draw objects without calling glBufferData for each frame */
    for (int i = 0; i < renderer->object_count; i++) {
        Object *object = renderer->objects[i];
        /* Collision detection */
        for (int j = i + 1; j < renderer->object_count; j++) {
	    if (object->shape->shape == RECTANGLE && renderer->objects[j]->shape->shape == RECTANGLE) {
                RectangularCollision(object, renderer->objects[j]);
	    } else if (renderer->objects[i]->shape->shape == CIRCLE && renderer->objects[j]->shape->shape == CIRCLE) {
	        if (CircularCollision(object, renderer->objects[j])) {
                    ElasticCollision(renderer->objects[i], renderer->objects[j]);
	        }
	    } else {
	        fprintf(stderr, "ERR_NO_COLLISION_FOUND\n");
	    }
	}

        /* Object shape */
        glBindVertexArray(renderer->vao);
	if (object->shape->shape == RECTANGLE) {
	    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_unit_square), indices_unit_square, GL_STATIC_DRAW); /* ebo */
            glBufferData(GL_ARRAY_BUFFER, sizeof(unit_square), unit_square, GL_STATIC_DRAW); /* vao */
	} else if (object->shape->shape == CIRCLE) {
	    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (sizeof(GLuint) * CIRCLE_QUALITY * 3) , &renderer->indices_circle[0], GL_STATIC_DRAW); /* ebo */
	    glBufferData(GL_ARRAY_BUFFER, (sizeof(GLfloat) * CIRCLE_QUALITY * 4), &renderer->unit_circle[0], GL_STATIC_DRAW); /* vao */
	} else{
	    fprintf(stderr, "ERR_VERTICES\n");
	}
		
	if (renderer->objects[i]->collision->collide_count > 0) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
        object->collision->collide_count = 0;
	memset(object->collision->collide, 0, sizeof(object->collision->collide));

	glUniform2f(renderer->move, object->position.x, object->position.y);
        switch (object->color)
	{
	    case BLACK:
	        glUniform3f(renderer->color, 0.0f, 0.0f, 0.0f);
		break;
	    case WHITE:
	        glUniform3f(renderer->color, 1.0f, 1.0f, 1.0f);
		break;
	    case YELLOW:
	        glUniform3f(renderer->color, 1.0f, 1.0f, 0.0f);
		break;
	    case GREEN:
	        glUniform3f(renderer->color, 0.0f, 1.0f, 0.0f);
		break;
	    case RED:
	        glUniform3f(renderer->color, 1.0f, 0.0f, 0.0f);
		break;
	    case BLUE:
	        glUniform3f(renderer->color, 0.0f, 0.0f, 1.0f);
		break;
	    case CYAN:
	        glUniform3f(renderer->color, 0.0f, 1.0f, 1.0f);
		break;
	    default:
	        glUniform3f(renderer->color, 1.0f, 0.0f, 1.0f);
		break;
	}


	glUniform1f(renderer->zoom, zoom_value);
	if (object->shape->shape == RECTANGLE) {	    
	    glUniform2f(renderer->model, object->shape->size.x, object->shape->size.y);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	} else if (object->shape->shape == CIRCLE) {
	    glUniform2f(renderer->model, object->shape->radius, object->shape->radius);
	    glDrawElements(GL_TRIANGLES, CIRCLE_QUALITY * 4, GL_UNSIGNED_INT, NULL);
	} else {
	    fprintf(stderr, "ERR_UNKOWN_SHAPE\n");
	}
	
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    zoom_value -= yoffset;
}
