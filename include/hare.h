#ifndef JUMPHARE_HARE_H
#define JUMPHARE_HARE_H

#include <stdbool.h>
#include "e/window.h"
#include "r/ro_types.h"
#include "collision.h"
#include "pixelparticles.h"

enum hare_state {
    HARE_GROUNDED,
    HARE_FALLING,
    HARE_JUMPING,
    HARE_DOUBLE_JUMP,
    HARE_SLEEPING,
    HARE_DEAD,
    HARE_NUM_STATES
};

typedef struct {
    eWindow *window_ref;
    const Collision *collision_ref;
    PixelParticles *particles_ref;
    
    enum hare_state state;
    vec2 pos;
    vec2 speed;
    bool looking_left;
    
    struct {
        float speed;  // [-1 : 1]
        bool jump;
    } in;
    
    struct {
        bool jump_action;
    } out;
    
    struct {
        RoSingle ro;
        RoParticle sleep_zzz_ro;

        enum hare_state prev_state;

        float last_input_time;

        float jump_time;

        float set_speed_x;
        float set_jump_time;

        float freeze_time;

        float animate_time;

        float emit_dirt_add;
        int emit_dirt_next_add;

        float add_airstroke_time;

        float sleep_time;
        float sleep_ro_time;
        int sleep_zzz_id;
        float sleep_zzz_next;
        int wake_up_cnt;
    } L;
} Hare;

Hare *hare_new(float pos_x, float pos_y, const Collision *collision, PixelParticles *particles, eWindow *window);

void hare_kill(Hare **self_ptr);

void hare_update(Hare *self, float dtime);

void hare_render(const Hare *self, const mat4 *cam_mat);

void hare_set_sleep(Hare *self, bool instant);

#endif //JUMPHARE_HARE_H
