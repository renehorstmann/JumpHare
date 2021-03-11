#include "e/e.h"
#include "r/r.h"
#include "u/u.h"

#include "camera.h"
#include "hud_camera.h"
#include "tiles.h"
#include "background.h"
#include "tilemap.h"
#include "level.h"
#include "hare.h"
#include "airstroke.h"
#include "dead.h"
#include "controller.h"
#include "camera_control.h"


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
    e_gui_init();

    // init r (render)
    r_render_init(e_window.window);

    // init systems
    camera_init();
    hud_camera_init();
    tiles_init();
    background_init();
    level_init();
    tilemap_init();
    tilemap_load_level("res/levels/level_01.png");
    controller_init();
    hare_init();
    airstroke_init();
    dead_init();
    camera_control_init();
    

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
    
    u_time += delta_time;

    // fixed update ps
    while(u_time > 0) {
        float fixed_time = 1.0 / UPDATES_PER_SECOND;
        u_time -= fixed_time;
        
        if(dead_is_dead())
            fixed_time = 0;  // stop while dead
        
        // e updates
        e_input_update();
    

        // simulate
        camera_update();
        hud_camera_update();
        background_update(fixed_time);
        tilemap_update(fixed_time);
        level_update(fixed_time);
        controller_update(fixed_time);
        hare_update(fixed_time);
        airstroke_update(fixed_time);
        camera_control_update(fixed_time);
    
    }
    dead_update(delta_time);
    
    
    // render
    
    background_render();
    tilemap_render_back();
    level_render();
    airstroke_render();
    hare_render();
    tilemap_render_front();
    dead_render();
    controller_render();


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


