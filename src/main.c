#include "e/simple.h"
#include "s/log.h"

#include "camera.h"
#include "tiles.h"
#include "level.h"

#define UPDATES_DELTA_TIME_MS 5

// this function will be called at the start of the app
static void init() {
    s_log_set_min_level(S_LOG_INFO);

    // init systems
    camera_init();      // camera for the level
    tiles_init();       // loads all tile textures

    // manages the gameplay (tilemap, hare, background, ...)
    level_init(1);

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_CXXDROID)
    e_window_set_screen_mode(E_WINDOW_MODE_FULLSCREEN);
#endif
}

// this functions is called either each frame or at a specific update/s time
static void update(float delta_time) {
    level_update(delta_time);

    static int cnt = 0;
    static su32 stime = 0;
}


// this function is calles each frame to render stuff, dtime is the time between frames
static void render(float dtime) {
    // camera only needs to be updated before rendering
    camera_update();

    const mat4 *hudcam_mat = &camera.matrices_p;
    // render
    level_render(hudcam_mat);
}


int main(int argc, char **argv) {
    e_simple_start("JumpHare", "Horsimann",
                   1.0f,   // startup block time (the time in which "Horsimann" is displayed at startup)
                   UPDATES_DELTA_TIME_MS,
                   init, update, render);
    return 0;
}
