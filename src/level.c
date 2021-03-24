#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "utilc/assume.h"
#include "background.h"
#include "tilemap.h"
#include "hare.h"
#include "airstroke.h"
#include "dirt_particles.h"
#include "dead.h"
#include "controller.h"
#include "camera_control.h"
#include "camera.h"
#include "level.h"

const static Color_s START_CODE = {{0, 0, 2, 0}};

static struct {
    rRoBatch borders_ro;
    int current_lvl;
    int state;
} L;

static void load_game() {
    vec2 start_pos;
    assume(tilemap_get_positions(&start_pos, 1, START_CODE, 1) == 1, "start not found");
    
    hare_init(start_pos.x, start_pos.y);
    airstroke_init();
    dirt_particles_init();
    camera_control_init();
}

static void unload_game() {
    hare_kill();
    airstroke_kill();
    dirt_particles_kill();
    camera_control_kill();
}

static void reset() {
    int state = L.state;
    unload_game();
    L.state = state;
    load_game();
}

static void dead_callback(void *ud) {
    reset();
}

void level_init(int lvl) {
    assume(lvl == 1, "...");

    L.current_lvl = lvl;


    background_init("res/backgrounds/greenhills.png");
    tilemap_init("res/levels/level_01.png");
    dead_init(dead_callback, NULL);
    controller_init();

    load_game();

    Color_s white_pixel = COLOR_WHITE;
    GLuint tex = r_texture_init(1, 1, white_pixel.v);
    r_ro_batch_init(&L.borders_ro, 4, camera.gl_main, tex);

    // black borders
    for (int i = 0; i < 4; i++) {
        L.borders_ro.rects[i].color = (vec4) {{0, 0, 0, 1}};
    }

    // border poses
    {
        float l = tilemap_border_left();
        float r = tilemap_border_right();
        float t = tilemap_border_top();
        float b = tilemap_border_bottom();
        float w = r - l;
        float h = t - b;
        L.borders_ro.rects[0].pose = u_pose_new_aa(
                l - 1024, t + 1024, 1024, h + 2048);
        L.borders_ro.rects[1].pose = u_pose_new_aa(
                l - 1024, t + 1024, w + 2048, 1024);
        L.borders_ro.rects[2].pose = u_pose_new_aa(
                r, t + 1024, 1024, h + 2048);
        L.borders_ro.rects[3].pose = u_pose_new_aa(
                l - 1024, b, w + 2048, 1024);

    }
    r_ro_batch_update(&L.borders_ro);
}

void level_kill() {
    background_kill();
    tilemap_kill();
    dead_kill();
    controller_kill();
    unload_game();

    r_ro_batch_kill(&L.borders_ro);
}

void level_update(float dtime) {
    dead_update(dtime);
    if (!dead_is_dead()) {
        background_update(dtime);
        tilemap_update(dtime);
        hare_update(dtime);
        airstroke_update(dtime);
        dirt_particles_update(dtime);
        controller_update(dtime);
    }
    camera_control_update(dtime);
}

void level_render() {
    background_render();
    tilemap_render_back();
    dirt_particles_render();
    airstroke_render();
    hare_render();
    tilemap_render_front();
    dead_render();

    r_ro_batch_render(&L.borders_ro);

    controller_render();
}

