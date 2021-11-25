#include "e/e.h"
#include "r/r.h"
#include "u/u.h"
#include "mathc/mathc.h"
#include "rhc/rhc.h"

#include "camera.h"
#include "hudcamera.h"
#include "tiles.h"
#include "level.h"

#define UPDATES_PER_SECOND 200

static struct {
    eWindow *window;
    eInput *input;
    eGui *gui;
    rRender *render;
    
    Camera_s *camera;
    HudCamera_s *hudcam;
    
    Tiles *tiles;
    Level *level;

    // debug text
    RoText fps_ro;
} L;

static float current_time() {
    return SDL_GetTicks() / 1000.0f;
}

static void main_loop(float delta_time);

int main(int argc, char **argv) {
    rhc_log_set_min_level(RHC_LOG_INFO);
    log_info("JumpHare");

    // init e (environment)
    L.window = e_window_new("JumpHare");
    L.input = e_input_new(L.window);
    L.gui = e_gui_new(L.window);       // nuklear debug windows
    ivec2 window_size = e_window_get_size(L.window);

    // init r (render)
    L.render = r_render_new(e_window_get_sdl_window(L.window));
    
    // the startup screen acts as loading screen and also checks for render errors
    r_render_show_startup(L.render,
            window_size.x, window_size.y,
            1.0, // block time
            "Horsimann");


    // init systems
    L.camera = camera_new();      // camera for the level
    L.hudcam = hudcamera_new();  // camera for hud elements
    L.tiles = tiles_new();       // loads all tile textures
    L.level = level_new(1, L.camera, L.hudcam, L.tiles, L.window, L.input, L.render);      // manages the gameplay (tilemap, hare, background, ...)

    // debug fps + load text
    L.fps_ro = ro_text_new_font55(64);
    for (int i = 0; i < L.fps_ro.ro.num; i++)
        L.fps_ro.ro.rects[i].color = (vec4) {{0, 0, 0, 1}};

#ifdef OPTION_GLES
    e_window_set_screen_mode(L.window, E_WINDOW_MODE_FULLSCREEN);
#endif

    e_window_main_loop(L.window, main_loop);

    r_render_kill(&L.render);
    e_gui_kill(&L.gui);
    e_input_kill(&L.input);
    e_window_kill(&L.window);

    return 0;
}


static void main_loop(float delta_time) {
    static float u_time = 0;

    //delta_time /= 5;
    ivec2 window_size = e_window_get_size(L.window);
    r_render_begin_frame(L.render, window_size.x, window_size.y);

    float start_time = current_time();

    // e updates
    e_input_update(L.input);

    // fixed update ps
    u_time += delta_time;
    while (u_time > 0) {
        const float fixed_time = 1.0 / UPDATES_PER_SECOND;
        u_time -= fixed_time;

        // simulate game
        level_update(L.level, fixed_time);
    }

    // camera only needs to be updated before rendering
    camera_update(L.camera, window_size.x, window_size.y);
    hudcamera_update(L.hudcam, window_size.x, window_size.y);
    
    const mat4 *hudcam_mat = &L.hudcam->matrices.p;

    // render
    level_render(L.level, L.camera, hudcam_mat);


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
        if (time > 0.25) {
            char text[64];
            sprintf(text, "%7.2f %3.0f%%", cnt / time, load_sum / cnt * 100);

            vec2 size = ro_text_set_text(&L.fps_ro, text);
            u_pose_set_xy(&L.fps_ro.pose,
                          sca_floor(-size.x / 2),
                          sca_floor(L.hudcam->RO.top - 2));
            time -= 0.25;
            cnt = 0;
            load_sum = 0;
        }
        ro_text_render(&L.fps_ro, hudcam_mat);
    }

    // nuklear debug windows
    e_gui_render(L.gui);

    // blit current frame to texture
    r_render_blit_framebuffer(L.render, window_size.x, window_size.y);

    // swap buffers
    r_render_end_frame(L.render);
}


