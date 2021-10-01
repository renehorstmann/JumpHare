#include "rhc/allocator.h"
#include "mathc/float.h"
#include "camera.h"
#include "tilemap.h"
#include "cameractrl.h"


//
// private
//
 

static void check_limits(CameraControl_s *self, const Camera_s *cam, const Tilemap *tilemap) {
    vec2 min, max;
    min.x = tilemap_border_left(tilemap) - cam->RO.left;
    max.x = tilemap_border_right(tilemap) - cam->RO.right;
    min.y = tilemap_border_bottom(tilemap) - cam->RO.bottom;
    max.y = tilemap_border_top(tilemap) - cam->RO.top;
    
    if (min.x > max.x) {
        min.x = max.x = (min.x + max.x) / 2;
    }
    if (min.y > max.y) {
        min.y = max.y = (min.y + max.y) / 2;
    }

    min = vec2_ceil(min);
    max = vec2_ceil(max);

    self->L.pos = vec2_clamp_vec(self->L.pos, min, max);
}

static void apply_pos(CameraControl_s *self, const Camera_s *cam, float dtime) {
    self->L.pos = vec2_add_vec(self->pos, cam->RO.offset);
}


//
// public
//

CameraControl_s *cameractrl_new() {
    CameraControl_s *self = rhc_calloc(sizeof *self);
    
    self->max_diff = vec2_set(20);
    self->diff_offset = (vec2) {{-10, 0}};
    
    return self;
}

void cameractrl_kill(CameraControl_s **self_ptr) {
    rhc_free(*self_ptr);
    *self_ptr = NULL;
}

void cameractrl_update(CameraControl_s *self, Camera_s *camera, const Tilemap *tilemap, float dtime) {
//	vec2_println(self->pos);
    vec2 delta = vec2_sub_vec(self->in.dst, self->pos);

    float max_right = self->diff_offset.x + self->max_diff.x;
    float max_left = self->diff_offset.x - self->max_diff.x;
    float max_top = self->diff_offset.x + self->max_diff.y;
    float max_bottom = self->diff_offset.x - self->max_diff.y;
    
    if (delta.x > max_right) {
        delta.x -= max_right;
        self->pos.x += delta.x;
    } else if(delta.x < max_left) {
        delta.x -= max_left;
        self->pos.x += delta.x;
    }
    
    if (delta.y > max_top) {
        delta.y -= max_top;
        self->pos.y += delta.y;
    } else if (delta.y < max_bottom) {
        delta.y -= max_bottom;
        self->pos.y += delta.y;
    }

    apply_pos(self, camera, dtime);

    check_limits(self, camera, tilemap);
    camera_set_pos(camera, self->L.pos.x, self->L.pos.y);
}

