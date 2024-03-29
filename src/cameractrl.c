#include "camera.h"
#include "tilemap.h"
#include "m/float.h"
#include "cameractrl.h"


struct CameraControl_Globals cameractrl;

static struct {
    vec2 pos;
} L;
 

static void check_limits() {
    vec2 min, max;
    min.x = tilemap_border_left() - camera.RO.left;
    max.x = tilemap_border_right() - camera.RO.right;
    min.y = tilemap_border_bottom() - camera.RO.bottom;
    max.y = tilemap_border_top() - camera.RO.top;
    
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
    L.pos = vec2_add_vec(cameractrl.pos, camera.RO.offset);
}


//
// public
//

void cameractrl_init() {
    cameractrl.max_diff = vec2_set(20);
    cameractrl.diff_offset = (vec2) {{-10, 0}};
}

void cameractrl_kill() {
    memset(&L, 0, sizeof L);
    memset(&cameractrl, 0, sizeof cameractrl);
}

void cameractrl_update(float dtime) {
//	vec2_println(cameractrl.pos);
    vec2 delta = vec2_sub_vec(cameractrl.in.dst, cameractrl.pos);

    float max_right = cameractrl.diff_offset.x + cameractrl.max_diff.x;
    float max_left = cameractrl.diff_offset.x - cameractrl.max_diff.x;
    float max_top = cameractrl.diff_offset.x + cameractrl.max_diff.y;
    float max_bottom = cameractrl.diff_offset.x - cameractrl.max_diff.y;
    
    if (delta.x > max_right) {
        delta.x -= max_right;
        cameractrl.pos.x += delta.x;
    } else if(delta.x < max_left) {
        delta.x -= max_left;
        cameractrl.pos.x += delta.x;
    }
    
    if (delta.y > max_top) {
        delta.y -= max_top;
        cameractrl.pos.y += delta.y;
    } else if (delta.y < max_bottom) {
        delta.y -= max_bottom;
        cameractrl.pos.y += delta.y;
    }

    apply_pos(dtime);

    check_limits();
    camera_set_pos(L.pos.x, L.pos.y);
}

