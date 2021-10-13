#ifndef JUMPHARE_CARROT_H
#define JUMPHARE_CARROT_H

#include "r/ro_types.h"
#include "pixelparticles.h"


typedef struct {
    PixelParticles *particles_ref;
    
    struct {
        int collected;  // eaten + available
        int eaten;
    } RO; // read only
    
    struct {
        RoBatch carrot_ro;
        bool collected[3];
        
        float time;

        struct {
            bool collected[3];
            int collected_cnt;
            int eaten_cnt;
        } save;
    } L;
} Carrot;

Carrot *carrot_new(PixelParticles *particles, const vec2 *positions_3);

void carrot_kill(Carrot **self_ptr);

void carrot_update(Carrot *self, float dtime);

void carrot_render(Carrot *self, const mat4 *cam_mat);

bool carrot_collect(Carrot *self, vec2 position);

void carrot_eat(Carrot *self);

void carrot_save(Carrot *self);

void carrot_load(Carrot *self);

#endif //JUMPHARE_CARROT_H
