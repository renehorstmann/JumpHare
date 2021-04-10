#ifndef R_RO_REFRACT_SINGLE_H
#define R_RO_REFRACT_SINGLE_H

#include <stdbool.h>
#include "core.h"
#include "rect.h"


// Renders a single rect in a draw call
typedef struct {
    rRect_s rect;
    const float *vp;
    GLuint program;
    GLuint vao;
    GLuint tex_main;
    GLuint tex_refraction;
    const GLuint *tex_framebuffer_ptr;
    bool owns_tex_main;
    bool owns_tex_refraction;
} rRoRefractSingle;

void r_ro_refract_single_init(rRoRefractSingle *self, const float *vp, 
        GLuint tex_main_sink, GLuint tex_refraction_sink, const GLuint *tex_framebuffer_ptr);

void r_ro_refract_single_kill(rRoRefractSingle *self);

void r_ro_refract_single_render(rRoRefractSingle *self);

void r_ro_refract_single_set_texture_main(rRoRefractSingle *self, GLuint tex_main_sink);

void r_ro_refract_single_set_texture_refraction(rRoRefractSingle *self, GLuint tex_refraction_sink);


#endif //R_RO_REFRACT_SINGLE_H
