#include "utilc/assume.h"
#include "background.h"
#include "tilemap.h"
#include "hare.h"
#include "airstroke.h"
#include "dead.h"
#include "controller.h"
#include "camera_control.h"
#include "level.h"

static struct {
   int current_lvl;
   int state;
} L;

static void reset() {
    int state = L.state;
    level_kill();
    L.state = state;
    level_init(L.current_lvl);
}

static void dead_callback(void *ud) {
    reset();
}

void level_init(int lvl) {
    assume(lvl==1, "...");

    L.current_lvl = lvl;


    background_init("res/backgrounds/greenhills.png");
    tilemap_init("res/levels/level_01.png");
    hare_init();
    airstroke_init();
    dead_init(dead_callback, NULL);
    controller_init();
    camera_control_init();
}

void level_kill() {
    background_kill();
    tilemap_kill();
    hare_kill();
    airstroke_kill();
    dead_kill();
    controller_kill();
    camera_control_kill();
}

void level_update(float dtime) {
    if(!dead_is_dead()) {
        background_update(dtime);
        tilemap_update(dtime);
        hare_update(dtime);
        airstroke_update(dtime);
        controller_update(dtime);
    }
    dead_update(dtime);
    camera_control_update(dtime);
}

void level_render() {
    background_render();
    tilemap_render_back();
    airstroke_render();
    hare_render();
    tilemap_render_front();
    dead_render();
    controller_render();
}

