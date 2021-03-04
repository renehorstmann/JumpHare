#include "mathc/float.h"
#include "r/ro_single.h"
#include "r/texture.h"
#include "e/input.h"
#include "u/pose.h"
#include "hare.h"
#include "hud_camera.h"
#include "controller.h"

#define DISTANCE 20
#define PULL_DISTANCE 40

static struct {
    rRoSingle background_ro;
    vec4 start_pos;
    vec4 actual_pos;
    vec4 last_pos;
} L;

static bool in_control_area(vec2 pos) {
    return u_pose_aa_contains(L.background_ro.rect.pose, pos);
}

static void pointer_event(ePointer_s pointer, void *ud) {
    L.actual_pos = pointer.pos;

    pointer.pos = mat4_mul_vec(hud_camera.matrices.p_inv, pointer.pos);


    if (!in_control_area(pointer.pos.xy)) {
        pointer.action = E_POINTER_UP;
    }

    if (pointer.action == E_POINTER_UP) {
        hare_set_speed(0);
        return;
    }

    if (pointer.action == E_POINTER_DOWN) {
        L.start_pos = pointer.pos;
        return;
    }

    float dx = pointer.pos.x - L.start_pos.x;
    if (sca_abs(dx) > PULL_DISTANCE) {
        float diff = sca_sign(dx) * (sca_abs(dx) - PULL_DISTANCE);
        L.start_pos.x += diff;
    }

    hare_set_speed(dx / DISTANCE);
}

static void key_ctrl() {
    static bool jumped = false;

    float speed_x = 0;
    if (e_input.keys.right && !e_input.keys.left)
        speed_x = 1;
    if (e_input.keys.left && !e_input.keys.right)
        speed_x = -1;
    hare_set_speed(speed_x);

    if (e_input.keys.space && !jumped) {
        jumped = true;
        hare_jump();
    }
    if (!e_input.keys.space)
        jumped = false;
}

void controller_init() {
#ifdef GLES
    e_input_register_pointer_event(pointer_event, NULL);
#endif

    r_ro_single_init(&L.background_ro, hud_camera.gl, r_texture_init_file("res/hud_background.png", NULL));
}

void controller_update(float dtime) {
#ifdef GLES
    float dy = L.actual_pos.y - L.last_pos.y;
    float speed = dy / dtime;
    if(speed>2)
        hare_jump();
#else
    key_ctrl();
#endif


    L.last_pos = L.actual_pos;

    if (hud_camera_is_portrait_mode()) {
        float w = hud_camera_width();
        float h = hud_camera_height() * HUD_CAMERA_SCREEN_WEIGHT;
        L.background_ro.rect.pose = u_pose_new(hud_camera_left() + w / 2, hud_camera_bottom() + h - w / 2, w, w);
    } else {
        float w = hud_camera_width() * HUD_CAMERA_SCREEN_WEIGHT;
        float h = hud_camera_height();
        L.background_ro.rect.pose = u_pose_new_angle(hud_camera_right() - w + h / 2, hud_camera_bottom() + h / 2, h, h,
                                                     M_PI_2);
    }
}

void controller_render() {
    r_ro_single_render(&L.background_ro);
}
