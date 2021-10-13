#ifndef JUMPHARE_GOAL_H
#define JUMPHARE_GOAL_H

#include <stdbool.h>
#include "r/ro_types.h"
#include "pixelparticles.h"

typedef struct {
    PixelParticles *particles_ref;
    
    struct {
        RoSingle goal_ro;
        float time;
    } L;
} Goal;

Goal *goal_new(PixelParticles *particles, vec2 position);

void goal_kill(Goal **self_ptr);

void goal_update(Goal *self, float dtime);

void goal_render(Goal *self, const mat4 *cam_mat);

bool goal_reached(const Goal *self);

vec2 goal_position(const Goal *self);

void goal_activate(Goal *self);

#endif //JUMPHARE_GOAL_H
