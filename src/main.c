#include "e/e.h"
#include "r/r.h"
#include "u/u.h"
#include "rhc/rhc.h"

#include "camera.h"
#include "hudcamera.h"
#include "tiles.h"
#include "level.h"

#define UPDATES_PER_SECOND 200

// debug text
static RoText fps_ro;

static float current_time() {
    return SDL_GetTicks() / 1000.0f;
}

static void main_loop(float delta_time);

int main(int argc, char **argv) {
    rhc_log_set_min_level(RHC_LOG_INFO);
    log_info("JumpHare");

    // init e (environment)
    e_window_init("JumpHare");
    e_input_init();
    e_gui_init();       // nuklear debug windows

    // init r (render)
    r_render_init(e_window.window);


    // init systems
    camera_init();      // camera for the level
    hudcamera_init();  // camera for hud elements
    tiles_init();       // loads all tile textures
    level_init(1);      // manages the gameplay (tilemap, hare, background, ...)

    // debug fps + load text
    fps_ro = ro_text_new_font55(64, hudcamera.gl);
    for(int i=0; i<fps_ro.ro.num; i++)
        fps_ro.ro.rects[i].color = (vec4) {{0, 0, 0, 1}};

#ifdef OPTION_GLES
    e_window_set_screen_mode(E_WINDOW_MODE_FULLSCREEN);
#endif
    
    e_window_main_loop(main_loop);

    e_gui_kill();

    return 0;
}


static void main_loop(float delta_time) {
    static float u_time = 0;

    //delta_time /= 5;

    r_render_begin_frame(e_window.size.x, e_window.size.y);

    float start_time = current_time();

    // e updates
    e_input_update();

    // fixed update ps
    u_time += delta_time;
    while (u_time > 0) {
        const float fixed_time = 1.0 / UPDATES_PER_SECOND;
        u_time -= fixed_time;

        // simulate game
        level_update(fixed_time);
    }

    // camera only needs to be updated before rendering
    camera_update();
    hudcamera_update();
    
    // render
    level_render();
    
    
    // fps + load
    {
        static float time = 0;
        static float load_sum = 0;
        static int cnt = 0;


        float frame_time = current_time() - start_time;
        float load = frame_time / delta_time;
        load_sum += load;

        time += delta_time;
        cnt++;
        if(time>0.25) {
            char text[64];
            sprintf(text, "%7.2f %3.0f%%", cnt/time, load_sum/cnt*100);
            
            vec2 size = ro_text_set_text(&fps_ro, text);
            u_pose_set_xy(&fps_ro.pose,
                          -size.x/2,
                          hudcamera_top()-2);
            time -= 0.25;
            cnt = 0;
            load_sum = 0;
        }
        ro_text_render(&fps_ro);
    }
    
    // nuklear debug windows
    e_gui_render();

    // blit current frame to texture
    r_render_blit_framebuffer(e_window.size.x, e_window.size.y);

    // swap buffers
    r_render_end_frame();
}


