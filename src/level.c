#include "utilc/assume.h"
#include "background.h"
#include "tilemap.h"
#include "hare.h"
#include "airstroke.h"
#include "dirt_particles.h"
#include "dead.h"
#include "controller.h"
#include "camera_control.h"
#include "level.h"

static struct {
   int current_lvl;
   int state;
} L;


static void load_game() {
	hare_init();
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
    assume(lvl==1, "...");

    L.current_lvl = lvl;


    background_init("res/backgrounds/greenhills.png");
    tilemap_init("res/levels/level_01.png");
    dead_init(dead_callback, NULL);
    controller_init();
    
    load_game();
}

void level_kill() {
    background_kill();
    tilemap_kill();
    dead_kill();
    controller_kill();
    unload_game();
}

void level_update(float dtime) {
    dead_update(dtime);
    if(!dead_is_dead()) {
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
    controller_render();
}

