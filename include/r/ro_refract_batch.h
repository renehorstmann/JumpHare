#ifndef R_RO_BATCH_H
#define R_RO_BATCH_H

#include <stdbool.h>
#include "core.h"
#include "rect.h"

// Renders multiple rects with a single draw call
// view_aabb is the screen space in which the rect is rendered
//    in texture space (origin is top left) [0:1]
//    as center_x, _y, radius_x, _y
//    defaults to fullscreen (0.5, 0.5, 0.5, 0.5)
typedef struct {
    rRect_s *rects;
    int num;
    const float *vp;
    const float *scale; // float
    const float *view_aabb; // vec4
    GLuint program;
    GLuint vao;
    GLuint vbo;
    GLuint tex_main;
    GLuint tex_refraction;
    const GLuint *tex_framebuffer_ptr;  // init as &r_render.framebuffer_tex
    bool owns_tex_main;
    bool owns_tex_refraction;
} rRoRefractBatch;

void r_ro_refract_batch_init(rRoRefractBatch *self, int num, 
        const float *vp, const float *scale_ptr,
        GLuint tex_main_sink, GLuint tex_refraction_sink);

void r_ro_refract_batch_kill(rRoRefractBatch *self);

void r_ro_refract_batch_update_sub(rRoRefractBatch *self, int offset, int size);

void r_ro_refract_batch_render_sub(rRoRefractBatch *self, int num);

void r_ro_refract_batch_set_texture_main(rRoRefractBatch *self, GLuint tex_main_sink);

void r_ro_refract_batch_set_texture_refractiob(rRoRefractBatch *self, GLuint tex_refraction_sink);


static void r_ro_refract_batch_update(rRoRefractBatch *self) {
    r_ro_refract_batch_update_sub(self, 0, self->num);
}

static void r_ro_refract_batch_render(rRoRefractBatch *self) {
    r_ro_refract_batch_render_sub(self, self->num);
}


#endif //R_RO_BATCH_H
