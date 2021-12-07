#include "e/simple.h"
#include "rhc/log.h"

#include "camera.h"
#include "hudcamera.h"
#include "tiles.h"
#include "level.h"

#define UPDATES_PER_SECOND 200

static struct {
    Camera_s *camera;
    HudCamera_s *hudcam;
    
    Tiles *tiles;
    Level *level;
} L;

// this function will be called at the start of the app
static void init(eSimple *simple, ivec2 window_size) {
    rhc_log_set_min_level(RHC_LOG_INFO);

    // init systems
    L.camera = camera_new();      // camera for the level
    L.hudcam = hudcamera_new();  // camera for hud elements
    L.tiles = tiles_new();       // loads all tile textures

    // manages the gameplay (tilemap, hare, background, ...)
    L.level = level_new(1, L.camera, L.hudcam, L.tiles,
                        simple->window, simple->input, simple->render);

#ifdef OPTION_GLES
    e_window_set_screen_mode(simple->window, E_WINDOW_MODE_FULLSCREEN);
#endif
}

// this functions is called either each frame or at a specific update/s time
static void update(eSimple *simple, ivec2 window_size, float delta_time) {
    level_update(L.level, delta_time);
}


// this function is calles each frame to render stuff, dtime is the time between frames
static void render(eSimple *simple, ivec2 window_size, float dtime) {
    // camera only needs to be updated before rendering
    camera_update(L.camera, window_size.x, window_size.y);
    hudcamera_update(L.hudcam, window_size.x, window_size.y);

    const mat4 *hudcam_mat = &L.hudcam->matrices.p;
    // render
    level_render(L.level, L.camera, hudcam_mat);
}


int main(int argc, char **argv) {
    e_simple_start("JumpHare", "Horsimann",
                   1.0f,   // startup block time (the time in which "Horsimann" is displayed at startup)
                   UPDATES_PER_SECOND,
                   init, update, render);
    return 0;
}
