#include "e/e.h"
#include "r/r.h"
#include "u/u.h"

#include "camera.h"
#include "hare.h"
#include "controller.h"


static void main_loop(float delta_time);


int main(int argc, char **argv) {
    SDL_Log("some");

    // init e (environment)
    e_window_init("some");
    e_input_init();
    e_gui_init();

    // init r (render)
    r_render_init(e_window.window);

    // init systems
    camera_init();
    hare_init();
    conttoller_init();
      

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
    hare_update(delta_time);
    

    // render
    r_render_begin_frame(e_window.size.x, e_window.size.y);


    hare_render();
    


    e_gui_render();

    // swap buffers
    r_render_end_frame();

    // check for opengl errors:
    r_render_error_check();
}


