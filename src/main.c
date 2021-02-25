#include "e/e.h"
#include "r/r.h"
#include "u/u.h"

#include "tiles.h"
#include "camera.h"
#include "background.h"
#include "tilemap.h"
#include "level.h"
#include "hare.h"
#include "controller.h"
#include "camera_control.h"


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
    tiles_init();
    camera_init();
    background_init();
    level_init();
    tilemap_init();
    tilemap_load_level("res/levels/level_01.png");
    controller_init();
    hare_init();
    camera_control_init();
    

    e_window_main_loop(main_loop);

    e_gui_kill();
    
    return 0;
}


static void main_loop(float delta_time) {
    // e updates
    e_window_update();
    e_input_update();
    

    // simulate
    camera_update();
    background_update(delta_time);
    tilemap_update(delta_time);
    level_update(delta_time);
    controller_update(delta_time);
    hare_update(delta_time);
    camera_control_update(delta_time);
    
    

    // render
    r_render_begin_frame(e_window.size.x, e_window.size.y);


    controller_render();
    background_render();
    tilemap_render_back();
    level_render();
    hare_render();
    tilemap_render_front();
    


    e_gui_render();

    // swap buffers
    r_render_end_frame();

    // check for opengl errors:
    r_render_error_check();
}


