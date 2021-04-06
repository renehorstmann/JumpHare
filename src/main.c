#include "e/e.h"
#include "r/r.h"
#include "u/u.h"

#include "camera.h"
#include "hud_camera.h"
#include "tiles.h"
#include "level.h"


#define UPDATES_PER_SECOND 200


static rRoText fps_ro;

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

    r_ro_text_init_font55(&fps_ro, 64, hud_camera.gl);
    for(int i=0; i<fps_ro.ro.num; i++)
        fps_ro.ro.rects[i].color = (vec4) {{0, 0, 0, 1}};

    e_window_main_loop(main_loop);

    e_gui_kill();

    return 0;
}


static void main_loop(float delta_time) {
    static float u_time = 0;

    //delta_time /= 5;

    r_render_begin_frame(e_window.size.x, e_window.size.y);

    float start_time = current_time();


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
    
    // fps + load
    {
        static float time = 0;
        static int cnt = 0;


        float frame_time = current_time() - start_time;
        float load = frame_time / delta_time;

        time += delta_time;
        cnt++;
        if(time>0.25) {
            char text[64];
            sprintf(text, "%7.2f %3.0f%%", cnt/time, load*100);
            vec2 size = r_ro_text_set_text(&fps_ro, text);
            u_pose_set_xy(&fps_ro.pose,
                          hud_camera_right() - size.x-2,
                          hud_camera_top()-2);
            time -= 0.25;
            cnt = 0;
        }
        r_ro_text_render(&fps_ro);
    }

    // nuklear debug windows
    e_gui_render();

    // swap buffers
    r_render_end_frame();

    // check for opengl errors:
    r_render_error_check();
}


