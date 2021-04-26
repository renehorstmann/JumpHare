#include "e/window.h"
#include "r/ro_single.h"
#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/int.h"
#include "mathc/utils/random.h"
#include "camera.h"
#include "tilemap.h"
#include "airstroke.h"
#include "dirtparticles.h"
#include "dead.h"
#include "tiles.h"
#include "collision.h"
#include "hare.h"

// debug:
#include "r/ro_text.h"
#include "hudcamera.h"

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
#define DOUBLE_JUMP_MAX_START_SPEED_Y 25
#define DOUBLE_JUMP_START_TIME 0.25
#define DOUBLE_JUMP_FREEZE_TIME 0.05
#define AIRSTROKE_DELAY_TIME 0.1

#define COLL_OFFSET_Y -3.0
#define COLL_RADIUS_X 7.0
#define COLL_RADIUS_Y 11.0

#define ANIMATION_FRAMES 8
#define ANIMATION_GROUNDED_FPS 12.0

#define SET_JUMP_TIME -0.05

#define NO_INPUT_SLEEP_TIME 10.0
#define FALLING_ASLEEP_TIME 0.2
#define WAKE_UP_TIME 0.5
#define WAKE_UP_SPEED 75
#define WAKE_UP_CNTS 3
#define SLEEP_ZZZ_MAX 16

const char *state_names[] = {
    "HARE_GROUNDED",
    "HARE_FALLING",
    "HARE_JUMPING",
    "HARE_DOUBLE_JUMP",
    "HARE_NUM_STATES"
};

static struct {
    RoSingle ro;
    RoParticle sleep_zzz_ro;
    RoText input_text;

    enum hare_state state, prev_state;

    vec2 pos, prev_pos;
    vec2 speed;
    float last_input_time;

    float jump_time;

    float set_speed_x;
    float set_jump_time;

    float freeze_time;

    float animate_time;
    bool looking_left;

    float emit_dirt_add;
    int emit_dirt_next_add;

    float add_airstroke_time;
    
    float sleep_time;
    float sleep_ro_time;
    int sleep_zzz_id;
    float sleep_zzz_next;
    int wake_up_cnt;
} L;


static void emit_zzz(bool exclamation) {
    rParticleRect_s *r = &L.sleep_zzz_ro.rects[L.sleep_zzz_id];
    
    r->pose = u_pose_new(
            L.pos.x + sca_random_noise(0, 5), 
            L.pos.y + sca_random_range(0, 10), 
            16, 16);
    
    r->color.rgb = vec3_random_range(0.9, 1.0);
    
    r->color.a = exclamation? 3:1;
    r->sprite.y = exclamation? 1:0;
    
    r->speed.x = sca_random_noise(0, 20);
    r->speed.y = 30;
    r->acc.x = r->speed.x * -0.25;
    r->acc.y = -0.25;
    r->color_speed.a = exclamation? -1.5 : -0.5;
    r->start_time = L.sleep_ro_time;
    
    ro_particle_update_sub(&L.sleep_zzz_ro, L.sleep_zzz_id, 1);
    L.sleep_zzz_id = (L.sleep_zzz_id+1) % L.sleep_zzz_ro.num;
}


static void check_fall_asleep(float dtime) {
    if(L.state != HARE_GROUNDED)
        return;
        
    L.last_input_time += dtime;
    if(L.last_input_time > NO_INPUT_SLEEP_TIME) {
        hare_set_sleep(false);
    }
}

static void check_wake_up(float dtime) {
    if(L.sleep_time != 0)
        return;
    if(
            (
                L.wake_up_cnt%2==0 
                && L.set_speed_x > WAKE_UP_SPEED
            )||( 
                L.wake_up_cnt%2==1 
                && L.set_speed_x < -WAKE_UP_SPEED
            )) {
        L.wake_up_cnt++;
        emit_zzz(true);
        L.sleep_zzz_next = 1.5;
    }
    if(L.wake_up_cnt >= WAKE_UP_CNTS) {
        L.sleep_time = WAKE_UP_TIME;
    }
}

static void check_jumping(float dtime) {
    if (L.set_jump_time <= 0) {
        if (L.state == HARE_GROUNDED) {
            L.state = HARE_JUMPING;
            L.jump_time = 0;
            L.set_jump_time = 1;
        }

        if (L.speed.y <= DOUBLE_JUMP_MAX_START_SPEED_Y
                && L.state == HARE_FALLING 
                && L.jump_time > DOUBLE_JUMP_START_TIME) {

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
        log_info("hare: dead by tilemap");
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
        case HARE_SLEEPING:
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
    if (L.speed.x < -MIN_SPEED_X)
        L.looking_left = true;
    if (L.speed.x > MIN_SPEED_X)
        L.looking_left = false;
        
    if (!L.looking_left)
        L.ro.rect.uv = u_pose_new(0, 0, 1, 1);
    else
        L.ro.rect.uv = u_pose_new(0, 0, -1, 1);
        
    
    if (L.state == HARE_GROUNDED) {
        float fps = ANIMATION_GROUNDED_FPS;
        int frames = ANIMATION_FRAMES;
        L.animate_time = sca_mod(L.animate_time + dtime, frames / fps);
        int u = L.animate_time * fps;
        int v = sca_abs(L.speed.x) < MIN_SPEED_X ? 0 : 1;
        if (sca_abs(L.speed.x) >= RUN_SPEED_X)
            v++;
            
        L.ro.rect.sprite = (vec2) {{u, v}};

    } else if (L.state == HARE_SLEEPING) {
        int u = 2;
        
        if(L.sleep_time == 0) {
            // sleeping
            L.sleep_zzz_next -= dtime;
            if(L.sleep_zzz_next < 0) {
                emit_zzz(false);
                L.sleep_zzz_next = sca_random_noise(1, 0.25);
            }
            
        } else if(L.sleep_time < 0) {
            // fall asleep
            L.sleep_time = sca_min(0, L.sleep_time + dtime);
            u = L.sleep_time < -FALLING_ASLEEP_TIME/2? 0 : 1;
        } else if(L.sleep_time > 0) {
            // wake up
            L.sleep_time = sca_max(0, L.sleep_time - dtime);
            
            u += 6 - L.sleep_time / WAKE_UP_TIME * 5;
            
            if(L.sleep_time == 0) {
                // awake
                L.state = HARE_GROUNDED;
                L.set_jump_time = 1; // not jump accidently at wake up
            }
        }
        
        int v = 5;
        L.ro.rect.sprite = (vec2) {{u, v}};
    } else {
        // falling, etc.
        int u = sca_floor(L.jump_time * 10);
        u = isca_clamp(u, 0, 6);
        if (L.speed.y < 0) {
            u = 7;
        }
        
        int v = L.state == HARE_DOUBLE_JUMP? 4 : 3;
        
        L.ro.rect.sprite = (vec2) {{u, v}};
    }
}


static void emit_dirt(float dtime) {
    if (L.state != HARE_GROUNDED || sca_abs(L.speed.x) < RUN_SPEED_X) {
        return;
    }

    uColor_s id;
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

    uColor_s col = tilemap_pixel_main(0, grab_pos.x, grab_pos.y);

    vec2 particle_pos = L.pos;
    particle_pos.y -= 7;
    vec2 particle_dir = {{-L.speed.x / 10, 12}};
    dirtparticles_add(particle_pos, particle_dir, col, add);
}


static void check_state_change(float dtime) {
    if(L.state != L.prev_state) {
        log_trace("hare: state changed from %s to %s", state_names[L.prev_state], state_names[L.state]);
    }
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


static void on_pause_callback(bool resume, void *ud) {
    log_info("hare: pause callback");
    if(resume && L.state == HARE_GROUNDED)
        hare_set_sleep(true);
}


void hare_init(float pos_x, float pos_y) {
    e_window_register_pause_callback(on_pause_callback, NULL);
    
    L.state = L.prev_state = HARE_FALLING;

    L.pos.x = pos_x;
    L.pos.y = pos_y;
    
    L.last_input_time = 0;

    L.emit_dirt_add = sca_random_noise(6, 2);

    L.ro = ro_single_new(camera.gl_main, r_texture_new_file(8, 6, "res/hare.png"));
    
    u_pose_set_size(&L.ro.rect.pose, 32, 32);
    
    L.sleep_zzz_ro = ro_particle_new(SLEEP_ZZZ_MAX, camera.gl_main, r_texture_new_file(1, 2, "res/sleep_zzz.png"));
    
    for(int i=0; i<L.sleep_zzz_ro.num; i++) {
        L.sleep_zzz_ro.rects[i].pose = u_pose_new_hidden();
    }
    ro_particle_update(&L.sleep_zzz_ro);

    

    L.input_text = ro_text_new_font55(64, hudcamera.gl);
    ro_text_set_color(&L.input_text, R_COLOR_BLACK);
}

void hare_kill() {
    e_window_unregister_pause_callback(on_pause_callback);
    ro_single_kill(&L.ro);
    ro_text_kill(&L.input_text);
    memset(&L, 0, sizeof(L));
}

void hare_update(float dtime) {
    L.prev_state = L.state;
    L.prev_pos = L.pos;
    
    L.sleep_ro_time += dtime;
    
    check_fall_asleep(dtime);
    
    if (L.state == HARE_SLEEPING) {
        check_wake_up(dtime);
    } else {
        check_jumping(dtime);

        if (L.freeze_time > 0) {
            L.freeze_time -= dtime;
            apply_speed(dtime / 4);
        } else {
            apply_speed(dtime);
        } 
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
    ro_particle_render(&L.sleep_zzz_ro, L.sleep_ro_time);
    //ro_text_render(&L.input_text);
}

enum hare_state hare_state() {
    return L.state;
}

vec2 hare_position() {
    return (vec2) {{L.pos.x, L.pos.y}};
}

vec2 hare_speed() {
    return (vec2) {{L.speed.x, L.speed.y}};
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
    if(dx != 0)
        L.last_input_time = 0;
}

void hare_jump() {
    L.set_jump_time = SET_JUMP_TIME;
    L.last_input_time = 0;
}

void hare_set_sleep(bool instant) {
    log_info("hare: set_sleep (instant? %i)", instant);
    L.state = HARE_SLEEPING;
    L.sleep_time = instant? 0 : -FALLING_ASLEEP_TIME;
    L.wake_up_cnt = 0;
    L.sleep_ro_time = 0;
    L.sleep_zzz_next = 0.75;
        
    for(int i=0; i<L.sleep_zzz_ro.num; i++) {
        L.sleep_zzz_ro.rects[i].pose = u_pose_new_hidden();
    }
    ro_particle_update(&L.sleep_zzz_ro);
}
