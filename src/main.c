#include "e/e.h"
#include "r/r.h"
#include "u/u.h"

#include "camera.h"
#include "hud_camera.h"
#include "tiles.h"
#include "level.h"


#define UPDATES_PER_SECOND 200

// #define TRACK_WORKLOAD


static float current_time() {
    return SDL_GetTicks() / 1000.0f;
}

static void main_loop(float delta_time);

int main(int argc, char **argv) {
    SDL_Log("JumpHare");

    // init e (environment)
    e_window_init("JumpHare");
    e_input_init();
    e_gui_init();       // nuklear debug windows

    // init r (render)
    r_render_init(e_window.window);

    // init systems
    camera_init();      // camera for the level
    hud_camera_init();  // camera for hud elements
    tiles_init();       // loads all tile textures
    level_init(1);      // manages the gameplay (tilemap, hare, background, ...)


    e_window_main_loop(main_loop);

    e_gui_kill();

    return 0;
}


static void main_loop(float delta_time) {
    static float u_time = 0;

    //delta_time /= 5;

    r_render_begin_frame(e_window.size.x, e_window.size.y);

#ifdef TRACK_WORKLOAD
    float start_time = current_time();
#endif


    // fixed update ps
    u_time += delta_time;
    while (u_time > 0) {
        const float fixed_time = 1.0 / UPDATES_PER_SECOND;
        u_time -= fixed_time;

        // e updates
        e_input_update();

        // simulate game
        level_update(fixed_time);
    }

    // camera only need to be updated before rendering
    camera_update();
    hud_camera_update();


    // render
    level_render();

    // nuklear debug windows
    e_gui_render();

#ifdef TRACK_WORKLOAD
    float frame_time = current_time() - start_time;
    float load = frame_time / delta_time;
    printf("load: %.4f\n", load);
#endif

    // swap buffers
    r_render_end_frame();

    // check for opengl errors:
    r_render_error_check();
}


