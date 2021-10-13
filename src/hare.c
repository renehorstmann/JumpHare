#include "r/ro_single.h"
#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/int.h"
#include "mathc/utils/random.h"
#include "camera.h"
#include "tilemap.h"
#include "pixelparticles.h"
#include "hare.h"


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

//
// private
//

const char *state_names[] = {
        "HARE_GROUNDED",
        "HARE_FALLING",
        "HARE_JUMPING",
        "HARE_DOUBLE_JUMP",
        "HARE_SLEEPING",
        "HARE_DEAD"
};
_Static_assert(sizeof(state_names) / sizeof(*state_names) == HARE_NUM_STATES, "");




static void emit_zzz(Hare *self, bool exclamation) {
    rParticleRect_s *r = &self->L.sleep_zzz_ro.rects[self->L.sleep_zzz_id];

    r->pose = u_pose_new(
            self->pos.x + sca_random_noise(0, 5),
            self->pos.y + sca_random_range(0, 10),
            16, 16);

    r->color.rgb = vec3_random_range(0.9, 1.0);

    r->color.a = exclamation ? 3 : 1;
    r->sprite.y = exclamation ? 1 : 0;

    r->speed.x = sca_random_noise(0, 20);
    r->speed.y = 30;
    r->acc.x = r->speed.x * -0.25;
    r->acc.y = -0.25;
    r->color_speed.a = exclamation ? -1.5 : -0.5;
    r->start_time = self->L.sleep_ro_time;

    ro_particle_update_sub(&self->L.sleep_zzz_ro, self->L.sleep_zzz_id, 1);
    self->L.sleep_zzz_id = (self->L.sleep_zzz_id + 1) % self->L.sleep_zzz_ro.num;
}


static void check_fall_asleep(Hare *self, float dtime) {
    if (self->state != HARE_GROUNDED)
        return;

    self->L.last_input_time += dtime;
    if (self->L.last_input_time > NO_INPUT_SLEEP_TIME) {
        hare_set_sleep(self, false);
    }
}

static void check_wake_up(Hare *self, float dtime) {
    if (self->L.sleep_time != 0)
        return;
    if (
            (
                    self->L.wake_up_cnt % 2 == 0
                    && self->L.set_speed_x > WAKE_UP_SPEED
            ) || (
                    self->L.wake_up_cnt % 2 == 1
                    && self->L.set_speed_x < -WAKE_UP_SPEED
            )) {
        self->L.wake_up_cnt++;
        emit_zzz(self, true);
        self->L.sleep_zzz_next = 1.5;
    }
    if (self->L.wake_up_cnt >= WAKE_UP_CNTS) {
        self->L.sleep_time = WAKE_UP_TIME;
    }
}

static void check_jumping(Hare *self, float dtime) {
    if (self->L.set_jump_time <= 0) {
        if (self->state == HARE_GROUNDED) {
            self->state = HARE_JUMPING;
            self->L.jump_time = 0;
            self->L.set_jump_time = 1;
        }

        if (self->speed.y <= DOUBLE_JUMP_MAX_START_SPEED_Y
            && self->state == HARE_FALLING
            && self->L.jump_time > DOUBLE_JUMP_START_TIME) {

            self->speed.y = sca_max(self->speed.y, DOUBLE_JUMP_SPEED_Y);
#ifndef GOD_MODE
            self->state = HARE_DOUBLE_JUMP;
            self->L.freeze_time = DOUBLE_JUMP_FREEZE_TIME;
            self->L.jump_time = 0;
#endif
        }
        self->L.set_jump_time += dtime;
    }

    self->L.jump_time += dtime;

    if (self->state == HARE_JUMPING) {
        if (self->L.jump_time >= JUMP_START_TIME && self->speed.y <= 0) {
            self->speed.y = JUMP_SPEED;
        }
        if (self->L.jump_time >= JUMP_FALL_TIME) {
            self->state = HARE_FALLING;
        }
    }

}


static void apply_speed(Hare *self, float dtime) {
    vec2 pos = self->pos;

    float set_speed_x = self->state == HARE_DOUBLE_JUMP ?
                        self->L.set_speed_x * DOUBLE_JUMP_SPEED_X_FACTOR
                                                       : self->L.set_speed_x;
    // x
    if (sca_sign(set_speed_x) * sca_sign(self->speed.x) == -1) {
        self->speed.x = 0;
    }

    float diff = set_speed_x - self->speed.x;
    if (diff > 0) {
        self->speed.x = sca_min(self->speed.x
                               + (self->speed.x < 0 ? DEACC : ACC)
                                 * dtime, set_speed_x);
    } else if (diff < 0) {
        self->speed.x = sca_max(self->speed.x
                               - (self->speed.x > 0 ? DEACC : ACC)
                                 * dtime, set_speed_x);
    }

    float actual_speed = sca_abs(self->speed.x) < MIN_SPEED_X ? 0 : self->speed.x;

    pos.x += actual_speed * dtime;


    // y
    if (self->state != HARE_GROUNDED) {
        self->speed.y += GRAVITY * dtime;
        self->speed.y = sca_clamp(self->speed.y, -MAX_SPEED_Y, MAX_SPEED_Y);
        pos.y += self->speed.y * dtime;
    }

    self->pos = pos;
}


static void collision_callback(vec2 delta, enum collision_state state, void *ud) {
    Hare *self = ud;
    
    if (state == COLLISION_FALLING && self->state == HARE_GROUNDED) {
        self->state = HARE_FALLING;
        self->speed.y = 0;
        return;
    } else if (state == COLLISION_KILL) {
#ifndef GOD_MODE
        log_info("hare: dead by tilemap");
        self->state = HARE_DEAD;
#endif
        return;
    }

    // prevent gluing ground
    if (self->state == HARE_JUMPING && state == COLLISION_BOTTOM)
        delta.y = sca_max(delta.y, 0);

    self->pos = vec2_add_vec(self->pos, delta);

    if (state == COLLISION_BOTTOM) {
        if (self->state != HARE_JUMPING)
            self->speed.y = 0;
        if (self->state == HARE_FALLING || self->state == HARE_DOUBLE_JUMP)
            self->state = HARE_GROUNDED;
    } else if (state == COLLISION_TOP) {
        self->speed.y = 0;
    } else if (state == COLLISION_LEFT || state == COLLISION_RIGHT) {
        self->speed.x = 0;
    }
}

static void check_collision(Hare *self) {
    CollisionCallback_s coll = {collision_callback, self};
    vec2 center = {{self->pos.x, self->pos.y + COLL_OFFSET_Y}};
    vec2 radius = {{COLL_RADIUS_X, COLL_RADIUS_Y}};
    vec2 speed = self->speed;

    switch (self->state) {
        case HARE_GROUNDED:
        case HARE_JUMPING:
        case HARE_SLEEPING:
            collision_tilemap_grounded(self->collision_ref, coll, center, radius, speed);
            break;
        case HARE_FALLING:
        case HARE_DOUBLE_JUMP:
            collision_tilemap_falling(self->collision_ref, coll, center, radius, speed);
            break;
        default:
            log_wtf("invalid hare state");
    }
}


static void animate(Hare *self, float dtime) {
    if (self->speed.x < -MIN_SPEED_X)
        self->looking_left = true;
    if (self->speed.x > MIN_SPEED_X)
        self->looking_left = false;

    if (!self->looking_left)
        self->L.ro.rect.uv = u_pose_new(0, 0, 1, 1);
    else
        self->L.ro.rect.uv = u_pose_new(0, 0, -1, 1);


    if (self->state == HARE_GROUNDED) {
        float fps = ANIMATION_GROUNDED_FPS;
        int frames = ANIMATION_FRAMES;
        self->L.animate_time = sca_mod(self->L.animate_time + dtime, frames / fps);
        int u = self->L.animate_time * fps;
        int v = sca_abs(self->speed.x) < MIN_SPEED_X ? 0 : 1;
        if (sca_abs(self->speed.x) >= RUN_SPEED_X)
            v++;

        self->L.ro.rect.sprite = (vec2) {{u, v}};

    } else if (self->state == HARE_SLEEPING) {
        int u = 2;

        if (self->L.sleep_time == 0) {
            // sleeping
            self->L.sleep_zzz_next -= dtime;
            if (self->L.sleep_zzz_next < 0) {
                emit_zzz(self, false);
                self->L.sleep_zzz_next = sca_random_noise(1, 0.25);
            }

        } else if (self->L.sleep_time < 0) {
            // fall asleep
            self->L.sleep_time = sca_min(0, self->L.sleep_time + dtime);
            u = self->L.sleep_time < -FALLING_ASLEEP_TIME / 2 ? 0 : 1;
        } else if (self->L.sleep_time > 0) {
            // wake up
            self->L.sleep_time = sca_max(0, self->L.sleep_time - dtime);

            u += 6 - self->L.sleep_time / WAKE_UP_TIME * 5;

            if (self->L.sleep_time == 0) {
                // awake
                self->state = HARE_GROUNDED;
                self->L.set_jump_time = 1; // not jump accidently at wake up
            }
        }

        int v = 5;
        self->L.ro.rect.sprite = (vec2) {{u, v}};
    } else {
        // falling, etc.
        int u = sca_floor(self->L.jump_time * 10);
        u = isca_clamp(u, 0, 6);
        if (self->speed.y < 0) {
            u = 7;
        }

        int v = self->state == HARE_DOUBLE_JUMP ? 4 : 3;

        self->L.ro.rect.sprite = (vec2) {{u, v}};
    }
}


static void emit_dirt(Hare *self, float dtime) {
    if (self->state != HARE_GROUNDED || sca_abs(self->speed.x) < RUN_SPEED_X) {
        return;
    }

    uColor_s id;
    float ground = tilemap_ground(
    self->collision_ref->tilemap_ref, 
    self->pos.x, self->pos.y, &id);
    if (tiles_get_state(
    self->collision_ref->tilemap_ref->tiles_ref,
    id) != TILES_PIXEL_SOLID_DIRTY) {
        return;
    }

    self->L.emit_dirt_add += sca_abs(self->speed.x) / 6.0 * dtime;


    int add = self->L.emit_dirt_add;
    if (add < self->L.emit_dirt_next_add)
        return;

    self->L.emit_dirt_add -= add;
    self->L.emit_dirt_next_add = sca_random_noise(6, 2);

    vec2 grab_pos = {{self->pos.x, ground}};
    grab_pos = vec2_random_noise_vec(grab_pos, vec2_set(5));

    uColor_s col = tilemap_pixel_main(
    self->collision_ref->tilemap_ref,
    0, grab_pos.x, grab_pos.y);

    vec2 particle_pos = self->pos;
    particle_pos.y -= 7;
    vec2 particle_dir = {{-self->speed.x / 10, 12}};
    pixelparticles_add_dirt(self->particles_ref, particle_pos, particle_dir, col, add);
}


static void check_state_change(Hare *self, float dtime) {
    assume(self->state >= 0 && self->state < HARE_NUM_STATES, "invalid state");
    if (self->state != self->L.prev_state) {
        log_trace("hare: state changed from %s to %s", state_names[self->L.prev_state], state_names[self->state]);
    }
    if (self->state == HARE_DOUBLE_JUMP && self->L.prev_state != HARE_DOUBLE_JUMP) {
        self->L.add_airstroke_time = AIRSTROKE_DELAY_TIME;
    }
    if (self->L.add_airstroke_time > 0) {
        self->L.add_airstroke_time -= dtime;
        if (self->L.add_airstroke_time <= 0) {
            self->out.jump_action = true;
        }
    }
}


static void on_pause_callback(bool resume, void *ud) {
    log_info("hare: pause callback");
    Hare *self = ud;
    if (!resume && self->state == HARE_GROUNDED)
        hare_set_sleep(self, false);
}


//
// public
//

Hare *hare_new(float pos_x, float pos_y, const Collision *collision, PixelParticles *particles, eWindow *window) {
    Hare *self = rhc_calloc(sizeof *self);
    
    self->window_ref = window;
    self->collision_ref = collision;
    self->particles_ref = particles;
    
    e_window_register_pause_callback(window, on_pause_callback, self);

    self->state = HARE_FALLING;

    self->pos.x = pos_x;
    self->pos.y = pos_y;

    self->L.last_input_time = 0;

    self->L.emit_dirt_add = sca_random_noise(6, 2);

    self->L.ro = ro_single_new(r_texture_new_file(8, 6, "res/hare.png"));

    u_pose_set_size(&self->L.ro.rect.pose, 32, 32);

    self->L.sleep_zzz_ro = ro_particle_new(SLEEP_ZZZ_MAX, r_texture_new_file(1, 2, "res/sleep_zzz.png"));

    for (int i = 0; i < self->L.sleep_zzz_ro.num; i++) {
        self->L.sleep_zzz_ro.rects[i].pose = u_pose_new_hidden();
    }
    ro_particle_update(&self->L.sleep_zzz_ro);

    
    return self;
}

void hare_kill(Hare **self_ptr) {
    Hare *self = *self_ptr;
    if(!self)
        return;
    e_window_unregister_pause_callback(self->window_ref, on_pause_callback);
    ro_single_kill(&self->L.ro);
    
    rhc_free(self);
    *self_ptr = NULL;
}

void hare_update(Hare *self, float dtime) {
    self->L.prev_state = self->state;

    self->L.sleep_ro_time += dtime;

    self->L.set_speed_x = sca_clamp(self->in.speed, -1, 1) * MAX_SPEED_X;
    if (self->in.speed != 0) {
        self->L.last_input_time = 0;
    }

    if (self->in.jump) {
        self->L.set_jump_time = SET_JUMP_TIME;
        self->L.last_input_time = 0;
    }

    self->in.speed = 0;
    self->in.jump = false;
    self->out.jump_action = false;

    check_fall_asleep(self, dtime);

    if (self->state == HARE_SLEEPING) {
        check_wake_up(self, dtime);
    } else {
        check_jumping(self, dtime);

        if (self->L.freeze_time > 0) {
            self->L.freeze_time -= dtime;
            apply_speed(self, dtime / 4);
        } else {
            apply_speed(self, dtime);
        }
    }

    check_collision(self);

    u_pose_set_xy(&self->L.ro.rect.pose, self->pos.x, self->pos.y);

    animate(self, dtime);

    emit_dirt(self, dtime);

    check_state_change(self, dtime);
}

void hare_render(Hare *self, const mat4 *cam_mat) {
    ro_single_render(&self->L.ro, cam_mat);
    ro_particle_render(&self->L.sleep_zzz_ro, self->L.sleep_ro_time, cam_mat);
}


void hare_set_sleep(Hare *self, bool instant) {
    log_info("hare: set_sleep (instant? %i)", instant);
    self->state = HARE_SLEEPING;
    self->L.sleep_time = instant ? 0 : -FALLING_ASLEEP_TIME;
    self->L.wake_up_cnt = 0;
    self->L.sleep_ro_time = 0;
    self->L.sleep_zzz_next = 0.75;

    for (int i = 0; i < self->L.sleep_zzz_ro.num; i++) {
        self->L.sleep_zzz_ro.rects[i].pose = u_pose_new_hidden();
    }
    ro_particle_update(&self->L.sleep_zzz_ro);
}
