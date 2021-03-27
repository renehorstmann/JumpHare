#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "utilc/assume.h"
#include "background.h"
#include "tilemap.h"
#include "hare.h"
#include "airstroke.h"
#include "carrot.h"
#include "flag.h"
#include "dirt_particles.h"
#include "dead.h"
#include "controller.h"
#include "camera_control.h"
#include "camera.h"
#include "level.h"

const static Color_s START_CODE = {{0, 0, 2, 0}};
const static Color_s CARROT_CODE = {{0, 0, 2, 1}};
const static Color_s FLAG_CODE = {{0, 0, 2, 2}};


static struct {
    rRoBatch borders_ro;
    int current_lvl;
    int state;
} L;

static void load_game() {
    vec2 start_pos;
    assume(tilemap_get_positions(&start_pos, 1, START_CODE, 1) == 1, "start not found");
    
    vec2 flag_pos = flag_active_position();
    if(!sca_isnan(flag_pos.x))
        start_pos = flag_pos;
    
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
    
    carrot_load();
}

static void check_carrot() {
    carrot_collect(hare_position());
    
    vec2 strokes[AIRSTROKE_MAX];
    int strokes_num = airstroke_positions(strokes, AIRSTROKE_MAX);
    for(int i=0; i<strokes_num; i++) {
        carrot_collect(strokes[i]);
    }
}

static void dead_callback(void *ud) {
    reset();
}

void level_init(int lvl) {
    assume(lvl == 1, "...");

    L.current_lvl = lvl;

    tilemap_init("res/levels/level_01.png");
    background_init(tilemap_width(), tilemap_height(),
            true, false,
            "res/backgrounds/greenhills.png");

    vec2 carrot_pos[3];
    assume(tilemap_get_positions(carrot_pos, 3, CARROT_CODE, 1) == 3, "level needs 3 carrots");
    carrot_init(carrot_pos);

    vec2 flag_pos[64];
    int flags = tilemap_get_positions(flag_pos, 64, FLAG_CODE, 1);
    flag_init(flag_pos, flags);

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
    carrot_kill();
    flag_kill();
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
        carrot_update(dtime);
        flag_update(dtime);
        hare_update(dtime);
        airstroke_update(dtime);
        dirt_particles_update(dtime);
        controller_update(dtime);

        check_carrot();
    }
    camera_control_update(dtime);
}

void level_render() {
    background_render();
    flag_render();
    tilemap_render_back();
    carrot_render();
    dirt_particles_render();
    airstroke_render();
    hare_render();
    tilemap_render_front();
    dead_render();

    r_ro_batch_render(&L.borders_ro);

    controller_render();
}

