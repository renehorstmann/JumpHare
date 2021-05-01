#include "r/ro_batch.h"
#include "r/ro_batchrefract.h"
#include "r/texture.h"
#include "u/pose.h"
#include "u/image.h"
#include "mathc/float.h"
#include "rhc/error.h"
#include "background.h"
#include "tilemap.h"
#include "hare.h"
#include "airstroke.h"
#include "goal.h"
#include "carrot.h"
#include "flag.h"
#include "speechbubble.h"
#include "butterfly.h"
#include "dirtparticles.h"
#include "dead.h"
#include "controller.h"
#include "cameractrl.h"
#include "camera.h"
#include "level.h"

#define CODE_LAYER 2


const static uColor_s START_CODE = {{0, 0, 1, 0}};
const static uColor_s GOAL_CODE = {{0, 0, 1, 8}};
const static uColor_s CARROT_CODE = {{0, 0, 1, 1}};
const static uColor_s FLAG_CODE = {{0, 0, 1, 2}};
const static uColor_s BUTTERFLY_CODE = {{0, 0, 1, 3}};
const static uColor_s SPEECHBUBBLE_0_CODE = {{0, 0, 1, 4}};
const static uColor_s SPEECHBUBBLE_1_CODE = {{0, 0, 1, 5}};
const static uColor_s SPEECHBUBBLE_2_CODE = {{0, 0, 1, 6}};


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
    log_info("level: flag activated");
    carrot_save();
    butterfly_save();
}

static void load_game() {
    vec2 start_pos;
    assume(tilemap_get_positions(&start_pos, 1, START_CODE, CODE_LAYER) == 1, "start not found");
    
    vec2 flag_pos = flag_active_position();
    if(!sca_isnan(flag_pos.x))
        start_pos = flag_pos;
    
    hare_init(start_pos.x, start_pos.y);
    airstroke_init();
    dirtparticles_init();
    cameractrl_init();
    
}

static void unload_game() {
    hare_kill();
    airstroke_kill();
    dirtparticles_kill();
    cameractrl_kill();
    
}

static void reset() {
    int state = L.state;
    unload_game();
    L.state = state;
    load_game();
    
    carrot_load();
    butterfly_load();
}

static void check_carrot() {
    carrot_collect(hare_position());
    
    vec2 strokes[AIRSTROKE_MAX];
    int strokes_num = airstroke_positions(strokes, AIRSTROKE_MAX);
    for(int i=0; i<strokes_num; i++) {
        carrot_collect(strokes[i]);
    }
}

static void check_butterfly() {
    butterfly_collect(hare_position());
    
    vec2 strokes[AIRSTROKE_MAX];
    int strokes_num = airstroke_positions(strokes, AIRSTROKE_MAX);
    for(int i=0; i<strokes_num; i++) {
        butterfly_collect(strokes[i]);
    }
}

static void dead_callback(void *ud) {
    reset();
}

void level_init(int lvl) {
    log_info("level: init lvl %i", lvl);
    assume(lvl == 1, "...");

    L.current_lvl = lvl;

    tilemap_init("res/levels/level_01.png");
    background_init(tilemap_width(), tilemap_height(),
            true, false,
            "res/backgrounds/greenhills.png"
            //true, true,
            //"res/backgrounds/blueblocks.png"
            );
            
    vec2 goal_pos;
    assume(tilemap_get_positions(&goal_pos, 1, GOAL_CODE, CODE_LAYER) == 1, "level needs 1 goal");
    goal_init(goal_pos);

    vec2 carrot_pos[3];
    assume(tilemap_get_positions(carrot_pos, 3, CARROT_CODE, CODE_LAYER) == 3, "level needs 3 carrots");
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
        speechbubble_init(&L.bubbles[L.bubbles_size++], bubble_pos, "HsF");
    }
    if(tilemap_get_positions(&bubble_pos, 1, SPEECHBUBBLE_1_CODE, CODE_LAYER)) {
        speechbubble_init(&L.bubbles[L.bubbles_size++], bubble_pos, "Cf=EH");
    }
    if(tilemap_get_positions(&bubble_pos, 1, SPEECHBUBBLE_2_CODE, CODE_LAYER)) {
        speechbubble_init(&L.bubbles[L.bubbles_size++], bubble_pos, "TBZ");
    }

    dead_init(dead_callback, NULL);
    controller_init();
    

    load_game();
    
    hare_set_sleep(true);

    L.borders_ro = ro_batch_new(4, camera.gl_main, r_texture_new_white_pixel());

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


    // test
    uImage img;
    rTexture tex_main, tex_refract;
    img = u_image_new_file(2, "res/ice_block.png");
    assume(u_image_valid(img), "wtf");
    tex_main = r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, 0));
    tex_refract = r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, 1));
    u_image_kill(&img);
    L.ice = ro_batchrefract_new(1, camera.gl_main, camera.gl_scale, tex_main, tex_refract);
    L.ice.view_aabb = camera.gl_view_aabb;
    for(int i=0; i<L.ice.num; i++) {
        L.ice.rects[i].pose = u_pose_new(260+16*i, 100, 32, 64);
        L.ice.rects[i].color.a=0.8;
    }
    ro_batchrefract_update(&L.ice);


    img = u_image_new_file(2, "res/mirror.png");
    assume(u_image_valid(img), "wtf");
    tex_main = r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, 0));
    tex_refract = r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, 1));
    u_image_kill(&img);
    L.mirror = ro_batchrefract_new(1, camera.gl_main, camera.gl_scale, tex_main, tex_refract);
    L.mirror.view_aabb = camera.gl_view_aabb;
    for(int i=0; i<L.mirror.num; i++) {
        L.mirror.rects[i].pose = u_pose_new(120+32*i, 100, 32, 64);
    }
    ro_batchrefract_update(&L.mirror);

}

void level_kill() {
    background_kill();
    tilemap_kill();
    goal_kill();
    carrot_kill();
    butterfly_kill();
    flag_kill();
    for(int i=0; i<L.bubbles_size; i++) {
        speechbubble_kill(&L.bubbles[i]);
    }
    dead_kill();
    controller_kill();
    unload_game();

    ro_batch_kill(&L.borders_ro);

    // test
    ro_batchrefract_kill(&L.ice);
    ro_batchrefract_kill(&L.mirror);
}

void level_update(float dtime) {  
    goal_update(dtime);
    dead_update(dtime);
    if (!dead_is_dead()) {
        background_update(dtime);
        tilemap_update(dtime);
        carrot_update(dtime);
        flag_update(dtime);
        for(int i=0; i<L.bubbles_size; i++) {
            speechbubble_update(&L.bubbles[i], dtime);
        }
        hare_update(dtime);
        airstroke_update(dtime);
        butterfly_update(dtime);
        dirtparticles_update(dtime);
        controller_update(dtime);

        check_carrot();
        check_butterfly();
    }
    cameractrl_update(dtime);
}

void level_render() {
    background_render();
    flag_render();
    goal_render();
    tilemap_render_back();

    // test
    //ro_batchrefract_render(&L.ice);
    //ro_batchrefract_render(&L.mirror);

    for(int i=0; i<L.bubbles_size; i++) {
        speechbubble_render(&L.bubbles[i]);
    }
    carrot_render();
    dirtparticles_render();
    airstroke_render();
    hare_render();
    butterfly_render();
    tilemap_render_front();
    dead_render();

    ro_batch_render(&L.borders_ro);

    carrot_render_hud();

    controller_render();
    
}

