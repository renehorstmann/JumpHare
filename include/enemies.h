#ifndef JUMPHARE_ENEMIES_H
#define JUMPHARE_ENEMIES_H

#include "r/ro_batch.h"

#define ENEMIES_MAX_TYPES 32

struct Enemies_Type;

typedef void (*enemies_type_update_fn)(struct Enemies_Type *self, float dtime);

struct Enemies_Type {
    RoBatch ro;
    enemies_type_update_fn update;
    float time;
} Enemies_Type;


void enemies_init();

void enemies_kill();

void enemies_update(float dtime);

void enemies_render(const mat4 *cam_mat);

void enemies_add_hedgehogs(const vec2 *positions, int n);

#endif //JUMPHARE_ENEMIES_H
