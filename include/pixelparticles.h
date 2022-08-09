#ifndef JUMPHARE_PIXELPARTICLES_H
#define JUMPHARE_PIXELPARTICLES_H

#include "s/s.h"
#include "u/color.h"
#include "m/types/float.h"

struct PixelParticles_Globals {
    su32 time;
};
extern struct PixelParticles_Globals pixelparticles;

void pixelparticles_init();

void pixelparticles_kill();

void pixelparticles_update(float dtime);

void pixelparticles_render(const mat4 *cam_mat);

void pixelparticles_add(const rParticleRect_s *particles, int n);

void pixelparticles_add_dirt(vec2 pos, vec2 dir, uColor_s color, int n);

#endif //JUMPHARE_PIXELPARTICLES_H
