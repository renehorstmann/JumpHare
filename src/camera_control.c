#include "mathc/float.h"
#include "camera.h"
#include "hare.h"
#include "tilemap.h"
#include "camera_control.h"

#define MAX_DIFF 20

struct CameraControlGlobals_s camera_control;

static struct {
    vec2 pos;
} L;

static void check_limits() {
    vec2 min, max;
    min.x = tilemap_border_left() - camera_left();
    max.x = tilemap_border_right() - camera_right();
    min.y = tilemap_border_bottom() - camera_bottom();
    max.y = tilemap_border_top() - camera_top();

    min = vec2_floor(min);
    max = vec2_floor(max);
    
    if (min.x > max.x) {
        min.x = max.x = (min.x + max.x) / 2;
    }
    if (min.y > max.y) {
        min.y = max.y = (min.y + max.y) / 2;
    }

    L.pos = vec2_clamp_vec(L.pos, min, max);
}

static void apply_pos(float dtime) {
    L.pos = vec2_add_vec(camera_control.pos, camera_center_offset());
}


void camera_control_init() {
    camera_control.pos = hare_position();
}

void camera_control_kill() {

}

void camera_control_update(float dtime) {
//	vec2_println(camera_control.pos);
    vec2 h = hare_position();
    vec2 delta = vec2_sub_vec(h, camera_control.pos);
    vec2 diff = vec2_abs(delta);
    diff = vec2_sub(diff, MAX_DIFF);

    if (diff.x > 0) {
        camera_control.pos.x += sca_sign(delta.x) * diff.x;
    }
    if (diff.y > 0) {
        camera_control.pos.y += sca_sign(delta.y) * diff.y;
    }

    apply_pos(dtime);

    check_limits();
    camera_set_pos(L.pos.x, L.pos.y);
}

