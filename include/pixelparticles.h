#ifndef JUMPHARE_PIXELPARTICLES_H
#define JUMPHARE_PIXELPARTICLES_H

#include "r/rect.h"
#include "u/color.h"
#include "r/ro_types.h"

typedef struct {
    float time;
    
    struct {
        RoParticle ro;
        int next;
    } L;
} PixelParticles;

PixelParticles *pixelparticles_new();

void pixelparticles_kill(PixelParticles **self_ptr);

void pixelparticles_update(PixelParticles *self, float dtime);

void pixelparticles_render(PixelParticles *self, const mat4 *cam_mat);

void pixelparticles_add(PixelParticles *self, const rParticleRect_s *particles, int n);

void pixelparticles_add_dirt(PixelParticles *self, vec2 pos, vec2 dir, uColor_s color, int n);

#endif //JUMPHARE_PIXELPARTICLES_H
