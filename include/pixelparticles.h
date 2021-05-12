#ifndef JUMPHARE_PIXELPARTICLES_H
#define JUMPHARE_PIXELPARTICLES_H

#include "r/rect.h"
#include "u/color.h"
#include "mathc/types/float.h"

struct PixelParticlesGlobals_s {
    float time;
};
extern struct PixelParticlesGlobals_s pixelparticles;

void pixelparticles_init();

void pixelparticles_kill();

void pixelparticles_update(float dtime);

void pixelparticles_render();

void pixelparticles_add(const rParticleRect_s *particles, int n);

void pixelparticles_add_dirt(vec2 pos, vec2 dir, uColor_s color, int n);

#endif //JUMPHARE_PIXELPARTICLES_H
