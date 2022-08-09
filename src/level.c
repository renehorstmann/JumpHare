#include "r/ro_batch.h"
#include "r/ro_batchrefract.h"
#include "u/pose.h"
#include "u/image.h"
#include "m/float.h"
#include "m/utils/random.h"
#include "scripts.h"
#include "speechbubble.h"
#include "carrot.h"
#include "butterfly.h"
#include "tilemap.h"
#include "enemies.h"
#include "flag.h"
#include "goal.h"
#include "hare.h"
#include "airstroke.h"
#include "cameractrl.h"
#include "background.h"
#include "hud.h"
#include "controller.h"
#include "camera.h"
#include "collision.h"
#include "pixelparticles.h"
#include "background.h"
#include "dead.h"
#include "level.h"

#define CODE_LAYER 2


static const uColor_s START_CODE = {{1, 0, 0, 255}};
static const uColor_s GOAL_CODE = {{1, 0, 1, 255}};
static const uColor_s CARROT_CODE = {{1, 1, 0, 255}};
static const uColor_s FLAG_CODE = {{1, 2, 0, 255}};
static const uColor_s BUTTERFLY_CODE = {{1, 3, 0, 255}};
static const uColor_s SPEECHBUBBLE_0_CODE = {{1, 4, 0, 255}};
static const uColor_s SPEECHBUBBLE_1_CODE = {{1, 5, 0, 255}};
static const uColor_s SPEECHBUBBLE_2_CODE = {{1, 6, 0, 255}};

// enemies
static const uColor_s ENEMY_HEDGHEHOG_CODE  = {{1, 0, 2, 255}};



static struct {
    RoBatch borders_ro;
    int current_lvl;
    SpeechBubble bubbles[3];
    int bubbles_size;
    int state;

    // test
    RoBatchRefract ice, mirror;
} L;

static void on_flag_activated_cb(vec2 pos, void *ud) {
    s_log_info("level: flag activated");
    carrot_save();
    butterfly_save();
}

static void add_enemies() {
    vec2 hedgehogs[64];
    int hedgehogs_num = tilemap_get_positions(hedgehogs, 64, ENEMY_HEDGHEHOG_CODE, CODE_LAYER);
    
    enemies_add_hedgehogs(hedgehogs, hedgehogs_num);
}

static void load_game() {
    
    vec2 start_pos;
    s_assume(tilemap_get_positions(&start_pos, 1, START_CODE, CODE_LAYER) == 1, "start not found");
    
    vec2 flag_pos = flag.RO.active_pos;
    if(!sca_isnan(flag_pos.x))
        start_pos = flag_pos;
    
    hare_init(start_pos.x, start_pos.y);

    enemies_init();
    add_enemies();
    
    airstroke_init();
    
    cameractrl_init();
}

static void unload_game() {
    enemies_kill();
    hare_kill();
    airstroke_kill();
    cameractrl_kill();
}

static void reset() {
    unload_game();
    load_game();
    
    carrot_load();
    butterfly_load();
}

static void dead_callback(void *ud) {
    reset();
}



//
// public
//

void level_init(int lvl) {
    s_log_info("level: init lvl %i", lvl);
    s_assume(lvl == 1, "...");

    L.current_lvl = lvl;

    tilemap_init("res/levels/lvl_01.png");
    background_init(
            tilemap_width(),
            tilemap_height(),
            true, false,
            "res/backgrounds/greenhills.png"
            //true, true,
            //"res/backgrounds/blueblocks.png"
            );
    
    pixelparticles_init();
    
    vec2 goal_pos;
    s_assume(tilemap_get_positions(&goal_pos, 1, GOAL_CODE, CODE_LAYER) == 1, "level needs 1 goal");
    goal_init(goal_pos);

    vec2 carrot_pos[3];
    s_assume(tilemap_get_positions(carrot_pos, 3, CARROT_CODE, CODE_LAYER) == 3, "level needs 3 carrots");
    carrot_init(carrot_pos);
    
    vec2 butterfly_pos[512];
    int butterflies = tilemap_get_positions(butterfly_pos, 512, BUTTERFLY_CODE, CODE_LAYER);
    butterfly_init(butterfly_pos, butterflies);

    vec2 flag_pos[64];
    int flags = tilemap_get_positions(flag_pos, 64, FLAG_CODE, CODE_LAYER);
    flag_init(flag_pos, flags);
    flag_register_callback(on_flag_activated_cb, NULL);
    
    L.bubbles_size=0;
    vec2 bubble_pos;
    if(tilemap_get_positions(&bubble_pos, 1, SPEECHBUBBLE_0_CODE, CODE_LAYER)) {
        L.bubbles[L.bubbles_size++] = speechbubble_new(bubble_pos, "HsF");
    }
    if(tilemap_get_positions( &bubble_pos, 1, SPEECHBUBBLE_1_CODE, CODE_LAYER)) {
        L.bubbles[L.bubbles_size++] = speechbubble_new(bubble_pos, "Cf=EH");
    }
    if(tilemap_get_positions( &bubble_pos, 1, SPEECHBUBBLE_2_CODE, CODE_LAYER)) {
        L.bubbles[L.bubbles_size++] = speechbubble_new(bubble_pos, "TBZ");
    }

    dead_init(dead_callback, NULL);
    controller_init();
    hud_init();
    collision_init();
    

    load_game();
    
    // level 1?
    hare_set_sleep( true);

    // ro
    L.borders_ro = ro_batch_new(4, r_texture_new_white_pixel());

    // black borders
    for (int i = 0; i < 4; i++) {
        L.borders_ro.rects[i].color = R_COLOR_BLACK;
    }

    // border poses
    {
        float l = tilemap_border_left();
        float r = tilemap_border_right();
        float t = tilemap_border_top();
        float b = tilemap_border_bottom();
        float w = r-l;
        float h = t-b;
        L.borders_ro.rects[0].pose = u_pose_new_aa(
                l - 1024, t + 1024, 1024, h + 2048);
        L.borders_ro.rects[1].pose = u_pose_new_aa(
                l - 1024, t + 1024, w + 2048, 1024);
        L.borders_ro.rects[2].pose = u_pose_new_aa(
                r, t + 1024, 1024, h + 2048);
        L.borders_ro.rects[3].pose = u_pose_new_aa(
                l - 1024, b, w + 2048, 1024);

    }
    ro_batch_update(&L.borders_ro);
}

void level_kill() {
        
    background_kill();
    tilemap_kill();
    pixelparticles_kill();
    goal_kill();
    carrot_kill();
    butterfly_kill();
    flag_kill();
    for(int i=0; i<L.bubbles_size; i++) {
        speechbubble_kill(&L.bubbles[i]);
    }
    dead_kill();
    controller_kill();
    hud_kill();
    
    
    unload_game();

    ro_batch_kill(&L.borders_ro);

    memset(&L, 0, sizeof L);
}

void level_update(float dtime) {
    goal_update(dtime);
    dead_update(dtime);
    hud_update(dtime);
    
    if (!dead_is_dead()) {
        
        // module linkage
        scripts_update(dtime);
        
        background_update(dtime);
        tilemap_update(dtime);
        carrot_update(dtime);
        flag_update(dtime);
        for(int i=0; i<L.bubbles_size; i++) {
            speechbubble_update(&L.bubbles[i], dtime, hare.pos);
        }
        enemies_update(dtime);
        
        airstroke_update(dtime);
        butterfly_update(dtime);
        pixelparticles_update(dtime);
        
    }
}

void level_render(const mat4 *hudcam_mat) {
    const mat4 *cam_main_mat = &camera.matrices_main.vp;
    
    background_render();
    flag_render(cam_main_mat);
    goal_render( cam_main_mat);
    tilemap_render_back(cam_main_mat);


    for(int i=0; i<L.bubbles_size; i++) {
        speechbubble_render(&L.bubbles[i], cam_main_mat);
    }
    carrot_render(cam_main_mat);
    pixelparticles_render(cam_main_mat);
    airstroke_render(cam_main_mat);
    //enemies_render();
    hare_render(cam_main_mat);
    butterfly_render(cam_main_mat);
    tilemap_render_front(cam_main_mat);
    hud_render(hudcam_mat);
    dead_render(cam_main_mat, hudcam_mat);

    ro_batch_render(&L.borders_ro, cam_main_mat, false);

    controller_render( hudcam_mat);
    
}

