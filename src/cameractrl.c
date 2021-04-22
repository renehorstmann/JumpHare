#include "mathc/float.h"
#include "camera.h"
#include "hare.h"
#include "tilemap.h"
#include "cameractrl.h"

#define MAX_DIFF 20

struct CameraControlGlobals_s cameractrl;

static struct {
    vec2 pos;
} L;

static void check_limits() {
    vec2 min, max;
    min.x = tilemap_border_left() - camera_left();
    max.x = tilemap_border_right() - camera_right();
    min.y = tilemap_border_bottom() - camera_bottom();
    max.y = tilemap_border_top() - camera_top();
    
    if (min.x > max.x) {
        min.x = max.x = (min.x + max.x) / 2;
    }
    if (min.y > max.y) {
        min.y = max.y = (min.y + max.y) / 2;
    }

    min = vec2_ceil(min);
    max = vec2_ceil(max);

    L.pos = vec2_clamp_vec(L.pos, min, max);
}

static void apply_pos(float dtime) {
    L.pos = vec2_add_vec(cameractrl.pos, camera_center_offset());
}


void cameractrl_init() {
    cameractrl.pos = hare_position();
}

void cameractrl_kill() {

}

void cameractrl_update(float dtime) {
//	vec2_println(cameractrl.pos);
    vec2 h = hare_position();
    vec2 delta = vec2_sub_vec(h, cameractrl.pos);
    vec2 diff = vec2_abs(delta);
    diff = vec2_sub(diff, MAX_DIFF);

    if (diff.x > 0) {
        cameractrl.pos.x += sca_sign(delta.x) * diff.x;
    }
    if (diff.y > 0) {
        cameractrl.pos.y += sca_sign(delta.y) * diff.y;
    }

    apply_pos(dtime);

    check_limits();
    camera_set_pos(L.pos.x, L.pos.y);
}

