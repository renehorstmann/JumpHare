#include <assert.h>
#include "mathc/float.h"
#include "tilemap.h"
#include "tiles.h"
#include "collision.h"


#define SCAN_SIZE 4.0
#define SCAN_SIZE_GROUNDED 8.0
#define SCANNER_DISTANCE 0.66

#define SLOPE_CHECK_DISTANCE 2.0
#define MAX_SLOPE_RATIO 4.5


//
// private
//

static bool scan_left(const Collision *self, CollisionCallback_s callback, float x, float y) {
    uColor_s id;
    float pos = tilemap_wall_left(self->tilemap_ref, x + SCAN_SIZE, y, &id);
    if (pos <= x)
        return false;

    enum tiles_pixel_state pixel = tiles_get_state(self->tilemap_ref->tiles_ref, id);

    if (pixel == TILES_PIXEL_ONEWAY_UP)
        return false;

    enum collision_state state = pixel == TILES_PIXEL_KILL ? COLLISION_KILL : COLLISION_LEFT;

    vec2 delta = {{pos - x, 0}};
    callback.cb(delta, state, callback.cb_user_data);
    return true;
}

static bool scan_right(const Collision *self, CollisionCallback_s callback, float x, float y) {
    uColor_s id;
    float pos = tilemap_wall_right(self->tilemap_ref, x - SCAN_SIZE, y, &id);
    if (pos >= x)
        return false;

    enum tiles_pixel_state pixel = tiles_get_state(self->tilemap_ref->tiles_ref, id);

    if (pixel == TILES_PIXEL_ONEWAY_UP)
        return false;

    enum collision_state state = pixel == TILES_PIXEL_KILL ? COLLISION_KILL : COLLISION_RIGHT;

    vec2 delta = {{pos - x, 0}};
    callback.cb(delta, state, callback.cb_user_data);
    return true;
}

static bool scan_top(const Collision *self, CollisionCallback_s callback, float x, float y) {
    uColor_s id;
    float pos = tilemap_ceiling(self->tilemap_ref, x, y - SCAN_SIZE, &id);
    if (pos >= y)
        return false;

    enum tiles_pixel_state pixel = tiles_get_state(self->tilemap_ref->tiles_ref, id);

    if (pixel == TILES_PIXEL_ONEWAY_UP)
        return false;

    enum collision_state state = pixel == TILES_PIXEL_KILL ? COLLISION_KILL : COLLISION_TOP;

    vec2 delta = {{0, pos - y}};
    callback.cb(delta, state, callback.cb_user_data);
    return true;
}

static bool scan_bottom_falling(const Collision *self, CollisionCallback_s callback, float x, float y, float speed_y) {
    uColor_s id;
    float pos = tilemap_ground(self->tilemap_ref, x, y + SCAN_SIZE, &id);
    if (pos <= y)
        return false;

    enum tiles_pixel_state pixel = tiles_get_state(self->tilemap_ref->tiles_ref, id);

    enum collision_state state = pixel == TILES_PIXEL_KILL ? COLLISION_KILL : COLLISION_BOTTOM;

    vec2 delta = {{0, pos - y}};
    callback.cb(delta, state, callback.cb_user_data);
    return true;
}

static void scan_bottom_grounded(const Collision *self, CollisionCallback_s callback, float x_a, float x_b, float y) {
    assert(x_a < x_b);
    uColor_s id_a, id_b;
    float pos_a = tilemap_ground(self->tilemap_ref, x_a, y + SCAN_SIZE_GROUNDED, &id_a);
    float pos_b = tilemap_ground(self->tilemap_ref, x_b, y + SCAN_SIZE_GROUNDED, &id_b);

    float min_y = y - SCAN_SIZE;

    if (pos_a < min_y && pos_b < min_y) {
        callback.cb((vec2) {{0}}, COLLISION_FALLING, callback.cb_user_data);
        return;
    }


    enum collision_state state;
    vec2 delta = {{0}};
    if (pos_a > pos_b) {
        enum tiles_pixel_state pixel = tiles_get_state(self->tilemap_ref->tiles_ref, id_a);
        state = pixel == TILES_PIXEL_KILL ? COLLISION_KILL : COLLISION_BOTTOM;
        delta.y = pos_a - y;

    } else {
        enum tiles_pixel_state pixel = tiles_get_state(self->tilemap_ref->tiles_ref, id_b);
        state = pixel == TILES_PIXEL_KILL ? COLLISION_KILL : COLLISION_BOTTOM;
        delta.y = pos_b - y;
    }

    callback.cb(delta, state, callback.cb_user_data);
}


//
// public
//

Collision *collision_new(const Tilemap *tilemap) {
    Collision *self = rhc_calloc(sizeof *self);
    
    self->tilemap_ref = tilemap;
    
    return self;
}

void collision_kill(Collision **self_ptr) {
    
    rhc_free(*self_ptr);
    *self_ptr = NULL;
}


void collision_tilemap_grounded(const Collision *self, CollisionCallback_s callback, vec2 center, vec2 radius, vec2 speed) {
    if (center.y < tilemap_border_bottom(self->tilemap_ref)) {
        callback.cb((vec2) {{0}}, COLLISION_KILL, callback.cb_user_data);
        return;
    }

    // top
    if (scan_top(self, callback,
                 center.x - radius.x * SCANNER_DISTANCE,
                 center.y + radius.y))
        return;

    if (scan_top(self, callback,
                 center.x + radius.x * SCANNER_DISTANCE,
                 center.y + radius.y))
        return;


    // left
    if (scan_left(self, callback, center.x - radius.x, center.y))
        return;

    if (scan_left(self, callback, center.x - radius.x, center.y + radius.y * SCANNER_DISTANCE))
        return;


    // right
    if (scan_right(self, callback, center.x + radius.x, center.y))
        return;

    if (scan_right(self, callback, center.x + radius.x, center.y + radius.y * SCANNER_DISTANCE))
        return;


    // bottom (always calls the callback)
    scan_bottom_grounded(self, callback,
                         center.x - radius.x * SCANNER_DISTANCE,
                         center.x + radius.x * SCANNER_DISTANCE,
                         center.y - radius.y);
}

void collision_tilemap_falling(const Collision *self, CollisionCallback_s callback, vec2 center, vec2 radius, vec2 speed) {

    // bottom
    if (speed.y < 0) {
        if (center.y < tilemap_border_bottom(self->tilemap_ref)) {
            callback.cb((vec2) {{0}}, COLLISION_KILL, callback.cb_user_data);
            return;
        }

        if (scan_bottom_falling(self, callback, center.x - radius.x * SCANNER_DISTANCE, center.y - radius.y, speed.y))
            return;

        if (scan_bottom_falling(self, callback, center.x + radius.x * SCANNER_DISTANCE, center.y - radius.y, speed.y))
            return;
    }


    // top
    if (speed.y > 0) {
        if (scan_top(self, callback,
                     center.x - radius.x * SCANNER_DISTANCE,
                     center.y + radius.y))
            return;

        if (scan_top(self, callback,
                     center.x + radius.x * SCANNER_DISTANCE,
                     center.y + radius.y))
            return;
    }


    // left
    if (scan_left(self, callback, center.x - radius.x, center.y - radius.y * SCANNER_DISTANCE))
        return;

    if (scan_left(self, callback, center.x - radius.x, center.y + radius.y * SCANNER_DISTANCE))
        return;


    // right
    if (scan_right(self, callback, center.x + radius.x, center.y - radius.y * SCANNER_DISTANCE))
        return;

    if (scan_right(self, callback, center.x + radius.x, center.y + radius.y * SCANNER_DISTANCE))
        return;

}
