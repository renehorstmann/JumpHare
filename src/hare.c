#include "r/ro_single.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/int.h"
#include "mathc/utils/random.h"
#include "camera.h"
#include "tilemap.h"
#include "airstroke.h"
#include "dirt_particles.h"
#include "dead.h"
#include "tiles.h"
#include "collision.h"
#include "hare.h"

// debug:
#include "r/ro_text.h"
#include "hud_camera.h"

// #define GOD_MODE

#define MIN_SPEED_X 5
#define RUN_SPEED_X 90
#define MAX_SPEED_X 100
#define MAX_SPEED_Y 250
#define JUMP_SPEED 225
#define DOUBLE_JUMP_SPEED_Y 175
#define DOUBLE_JUMP_SPEED_X_FACTOR 0.5
#define ACC 160
#define DEACC 240
#define GRAVITY -400

#define SLIP_BEGIN_SLOPE 1.5
#define SLIP_END_SLOPE 0.75
#define SLIP_SPEED 50

#define JUMP_START_TIME 0.1
#define JUMP_FALL_TIME 0.2
#define DOUBLE_JUMP_START_TIME 0.25
#define DOUBLE_JUMP_FREEZE_TIME 0.05
#define AIRSTROKE_DELAY_TIME 0.1

#define COLL_OFFSET_Y -3.0
#define COLL_RADIUS_X 7.0
#define COLL_RADIUS_Y 11.0

#define ANIMATION_FRAMES 8
#define ANIMATION_GROUNDED_FPS 12.0

#define SET_JUMP_TIME -0.05

static struct {
    RoSingle ro;
    RoText input_text;

    enum hare_state state, prev_state;

    vec2 pos, prev_pos;
    vec2 speed;

    float jump_time;

    float set_speed_x;
    float set_jump_time;

    float freeze_time;

    float animate_time;
    bool looking_left;

    float emit_dirt_add;
    int emit_dirt_next_add;

    float add_airstroke_time;
} L;


static void check_jumping(float dtime) {
    if (L.set_jump_time <= 0) {
        if (L.state == HARE_GROUNDED) {
            L.state = HARE_JUMPING;
            L.jump_time = 0;
            L.set_jump_time = 1;
        }

        if (L.state == HARE_FALLING && L.jump_time > DOUBLE_JUMP_START_TIME) {

            L.speed.y = sca_max(L.speed.y, DOUBLE_JUMP_SPEED_Y);
#ifndef GOD_MODE
            L.state = HARE_DOUBLE_JUMP;
            L.freeze_time = DOUBLE_JUMP_FREEZE_TIME;
            L.jump_time = 0;
#endif
        }
        L.set_jump_time += dtime;
    }

    L.jump_time += dtime;

    if (L.state == HARE_JUMPING) {
        if (L.jump_time >= JUMP_START_TIME && L.speed.y <= 0) {
            L.speed.y = JUMP_SPEED;
        }
        if (L.jump_time >= JUMP_FALL_TIME) {
            L.state = HARE_FALLING;
        }
    }

}


static void apply_speed(float dtime) {
    vec2 pos = L.pos;

    float set_speed_x = L.state == HARE_DOUBLE_JUMP ?
                        L.set_speed_x * DOUBLE_JUMP_SPEED_X_FACTOR
                                                    : L.set_speed_x;
    // x
    if (sca_sign(set_speed_x) * sca_sign(L.speed.x) == -1) {
        L.speed.x = 0;
    }
    float diff = set_speed_x - L.speed.x;
    if (diff > 0) {
        L.speed.x = sca_min(L.speed.x
                            + (L.speed.x < 0 ? DEACC : ACC)
                              * dtime, set_speed_x);
    } else if (diff < 0) {
        L.speed.x = sca_max(L.speed.x
                            - (L.speed.x > 0 ? DEACC : ACC)
                              * dtime, set_speed_x);
    }

    float actual_speed = sca_abs(L.speed.x) < MIN_SPEED_X ? 0 : L.speed.x;

    pos.x += actual_speed * dtime;


    // y
    if (L.state != HARE_GROUNDED) {
        L.speed.y += GRAVITY * dtime;
        L.speed.y = sca_clamp(L.speed.y, -MAX_SPEED_Y, MAX_SPEED_Y);
        pos.y += L.speed.y * dtime;
    }

    L.pos = pos;
}


static void collision_callback(vec2 delta, enum collision_state state, void *ud) {
    if (state == COLLISION_FALLING && L.state == HARE_GROUNDED) {
        L.state = HARE_FALLING;
        L.speed.y = 0;
        return;
    } else if (state == COLLISION_KILL) {
#ifndef GOD_MODE
        dead_set_dead(L.pos.x, L.pos.y);
#endif
        return;
    }

    // prevent gluing ground
    if (L.state == HARE_JUMPING && state == COLLISION_BOTTOM)
        delta.y = sca_max(delta.y, 0);

    L.pos = vec2_add_vec(L.pos, delta);

    if (state == COLLISION_BOTTOM) {
        if (L.state != HARE_JUMPING)
            L.speed.y = 0;
        if (L.state == HARE_FALLING || L.state == HARE_DOUBLE_JUMP)
            L.state = HARE_GROUNDED;
    } else if (state == COLLISION_TOP) {
        L.speed.y = 0;
    } else if (state == COLLISION_LEFT || state == COLLISION_RIGHT) {
        L.speed.x = 0;
    }
}

static void check_collision() {
    Collision_s coll = {collision_callback};
    vec2 center = {{L.pos.x, L.pos.y + COLL_OFFSET_Y}};
    vec2 radius = {{COLL_RADIUS_X, COLL_RADIUS_Y}};
    vec2 speed = L.speed;

    switch (L.state) {
        case HARE_GROUNDED:
        case HARE_JUMPING:
            collision_tilemap_grounded(coll, center, radius, speed);
            break;
        case HARE_FALLING:
        case HARE_DOUBLE_JUMP:
            collision_tilemap_falling(coll, center, radius, speed);
            break;
        default:
            assert(0 && "invalid hare state");
    }
}


static void animate(float dtime) {
    int frame;
    if (L.state == HARE_GROUNDED) {
        float fps = ANIMATION_GROUNDED_FPS;
        int frames = ANIMATION_FRAMES;
        L.animate_time = sca_mod(L.animate_time + dtime, frames / fps);
        frame = L.animate_time * fps;

    } else {
        // not grounded
        frame = (int) sca_floor(L.jump_time * 10);
        frame = isca_clamp(frame, 0, 6);

        if (L.speed.y < 0) {
            frame = 7;
        }
    }

    float w = 1.0 / ANIMATION_FRAMES;
    float h = 1.0 / 5.0;

    if (L.speed.x < -MIN_SPEED_X)
        L.looking_left = true;
    if (L.speed.x > MIN_SPEED_X)
        L.looking_left = false;

    float v;
    if (L.state == HARE_GROUNDED) {
        v = sca_abs(L.speed.x) < MIN_SPEED_X ? 0 : 1;
        if (sca_abs(L.speed.x) >= RUN_SPEED_X)
            v++;
    } else if (L.state == HARE_DOUBLE_JUMP) {
        v = 4;
    } else {
        v = 3;
    }

    if (!L.looking_left)
        L.ro.rect.uv = u_pose_new(frame * w, v * h, w, h);
    else
        L.ro.rect.uv = u_pose_new((1 + frame) * w, v * h, -w, h);
}


static void emit_dirt(float dtime) {
    if (L.state != HARE_GROUNDED || sca_abs(L.speed.x) < RUN_SPEED_X) {
        return;
    }

    Color_s id;
    float ground = tilemap_ground(L.pos.x, L.pos.y, &id);
    if (tiles_get_state(id) != TILES_PIXEL_SOLID_DIRTY) {
        return;
    }

    L.emit_dirt_add += sca_abs(L.speed.x) / 6.0 * dtime;


    int add = L.emit_dirt_add;
    if (add < L.emit_dirt_next_add)
        return;

    L.emit_dirt_add -= add;
    L.emit_dirt_next_add = sca_random_noise(6, 2);

    vec2 grab_pos = {{L.pos.x, ground}};
    grab_pos = vec2_random_noise_vec(grab_pos, vec2_set(5));

    Color_s col = tilemap_pixel_main(0, grab_pos.x, grab_pos.y);

    vec2 particle_pos = L.pos;
    particle_pos.y -= 7;
    vec2 particle_dir = {{-L.speed.x / 10, 12}};
    dirt_particles_add(particle_pos, particle_dir, col, add);
}


static void check_state_change(float dtime) {
    if (L.state == HARE_DOUBLE_JUMP && L.prev_state != HARE_DOUBLE_JUMP) {
        L.add_airstroke_time = AIRSTROKE_DELAY_TIME;
    }
    if (L.add_airstroke_time > 0) {
        L.add_airstroke_time -= dtime;
        if (L.add_airstroke_time <= 0) {
            airstroke_add(L.pos.x, L.pos.y);
        }
    }
}


void hare_init(float pos_x, float pos_y) {
    L.state = L.prev_state = HARE_FALLING;

    L.pos.x = pos_x;
    L.pos.y = pos_y;

    L.emit_dirt_add = sca_random_noise(6, 2);

    ro_single_init(&L.ro, camera.gl_main, r_texture_new_file("res/hare.png", NULL));

    u_pose_set_size(&L.ro.rect.pose, 32, 32);


    ro_text_init_font55(&L.input_text, 64, hud_camera.gl);
    for (int i = 0; i < 64; i++) {
        L.input_text.ro.rects[i].color = (vec4) {{0, 0, 0, 1}};
    }
}

void hare_kill() {
    ro_single_kill(&L.ro);
    ro_text_kill(&L.input_text);
    memset(&L, 0, sizeof(L));
}

void hare_update(float dtime) {
    L.prev_state = L.state;
    L.prev_pos = L.pos;

    check_jumping(dtime);

    if (L.freeze_time > 0) {
        L.freeze_time -= dtime;
        apply_speed(dtime / 4);
    } else {
        apply_speed(dtime);
    }

    check_collision();

    u_pose_set_xy(&L.ro.rect.pose, L.pos.x, L.pos.y);
    animate(dtime);

    emit_dirt(dtime);

    check_state_change(dtime);

    // debug
    char text[64];
    sprintf(text, "%i %+08.3f %+05.3f\n  %+08.3f %+08.3f",
            L.state,
            L.set_speed_x, L.set_jump_time,
            L.speed.x, L.speed.y);
    ro_text_set_text(&L.input_text, text);
    u_pose_set_xy(&L.input_text.pose, camera_left(), camera_top());
}

void hare_render() {
    ro_single_render(&L.ro);
    //ro_text_render(&L.input_text);
}

enum hare_state hare_state() {
    return L.state;
}

vec2 hare_position() {
    return (vec2) {{L.pos.x, L.pos.y}};
}

vec2 hare_prev_position() {
    return (vec2) {{L.prev_pos.x, L.prev_pos.y}};
}

bool hare_looking_left() {
    return L.looking_left;
}

// [-1 : 1]
void hare_set_speed(float dx) {
    L.set_speed_x = sca_clamp(dx, -1, 1) * MAX_SPEED_X;
}

void hare_jump() {
    L.set_jump_time = SET_JUMP_TIME;
}
