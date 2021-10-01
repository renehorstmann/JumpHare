#include "r/ro_batch.h"
#include "r/ro_batchrefract.h"
#include "r/texture.h"
#include "u/pose.h"
#include "u/image.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "rhc/error.h"
#include "background.h"
#include "tilemap.h"
#include "enemies.h"
#include "hare.h"
#include "airstroke.h"
#include "goal.h"
#include "carrot.h"
#include "flag.h"
#include "speechbubble.h"
#include "butterfly.h"
#include "pixelparticles.h"
#include "dead.h"
#include "hud.h"
#include "controller.h"
#include "cameractrl.h"
#include "scripts.h"
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

// enemies
const static uColor_s ENEMY_HEDGHEHOG_CODE  = {{0, 0, 1, 16}};



//
// private
//

static void on_flag_activated_cb(vec2 pos, void *ud) {
    Level *self = ud;
    log_info("level: flag activated");
    carrot_save(self->carrot);
    butterfly_save(self->butterfly);
}

static void add_enemies() {
    vec2 hedgehogs[64];
    int hedgehogs_num = tilemap_get_positions(hedgehogs, 64, ENEMY_HEDGHEHOG_CODE, CODE_LAYER);
    
    enemies_add_hedgehogs(hedgehogs, hedgehogs_num);
}

static void load_game() {
    enemies_init();
    add_enemies();
    
    vec2 start_pos;
    assume(tilemap_get_positions(&start_pos, 1, START_CODE, CODE_LAYER) == 1, "start not found");
    
    vec2 flag_pos = flag_active_position();
    if(!sca_isnan(flag_pos.x))
        start_pos = flag_pos;
    
    hare_init(start_pos.x, start_pos.y, L.window);
    airstroke_init();
    pixelparticles_init();
    cameractrl_init();
}

static void unload_game() {
    enemies_kill();
    hare_kill();
    airstroke_kill();
    pixelparticles_kill();
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

static void dead_callback(void *ud) {
    reset();
}



//
// public
//

Level *level_new(int lvl, const Camera_s *cam, const HudCamera_s *hudcam, const Tiles *tiles, eWindow *window, eInput *input, rRender *render) {
    Level *self = rhc_calloc(sizeof *self);
    
    self->L.window_ref = window;
    log_info("level: init lvl %i", lvl);
    assume(lvl == 1, "...");

    self->L.current_lvl = lvl;

    self->tilemap = tilemap_new(tiles, "res/levels/level_01.png");
    self->background = background_new(
            tilemap_width(self->tilemap), 
            tilemap_height(self->tilemap),
            true, false,
            render,
            "res/backgrounds/greenhills.png"
            //true, true,
            //"res/backgrounds/blueblocks.png"
            );
            
    vec2 goal_pos;
    assume(tilemap_get_positions(self->tilemap, &goal_pos, 1, GOAL_CODE, CODE_LAYER) == 1, "level needs 1 goal");
    self->goal = goal_new(goal_pos);

    vec2 carrot_pos[3];
    assume(tilemap_get_positions(self->tilemap, carrot_pos, 3, CARROT_CODE, CODE_LAYER) == 3, "level needs 3 carrots");
    self->carrot = carrot_new(carrot_pos);
    
    vec2 butterfly_pos[512];
    int butterflies = tilemap_get_positions(self->tilemap, butterfly_pos, 512, BUTTERFLY_CODE, CODE_LAYER);
    self->butterfly = butterfly_new(butterfly_pos, butterflies);

    vec2 flag_pos[64];
    int flags = tilemap_get_positions(self->tilemap, flag_pos, 64, FLAG_CODE, CODE_LAYER);
    self->flag = flag_new(flag_pos, flags, cam, self->carrot, input);
    flag_register_callback(self->flag, on_flag_activated_cb, self);
    
    self->L.bubbles_size=0;
    vec2 bubble_pos;
    if(tilemap_get_positions(self->tilemap, &bubble_pos, 1, SPEECHBUBBLE_0_CODE, CODE_LAYER)) {
        self->L.bubbles[self->L.bubbles_size++] = speechbubble_new(bubble_pos, "HsF");
    }
    if(tilemap_get_positions(self->tilemap, &bubble_pos, 1, SPEECHBUBBLE_1_CODE, CODE_LAYER)) {
        self->L.bubbles[self->L.bubbles_size++] = speechbubble_new(bubble_pos, "Cf=EH");
    }
    if(tilemap_get_positions(self->tilemap, &bubble_pos, 1, SPEECHBUBBLE_2_CODE, CODE_LAYER)) {
        self->L.bubbles[self->L.bubbles_size++] = speechbubble_new(bubble_pos, "TBZ");
    }

    self->dead = dead_new(dead_callback, NULL);
    self->controller = controller_new(input, cam, hudcam);
    self->hud = hud_new();
    self->scripts = scripts_new(
    self->controller,
    cam,
    self->camctrl,
    self->airstroke,
    self->butterfly,
    self->carrot
    );

    load_game();
    
    // level 1?
    hare_set_sleep(true);

    // ro
    self->L.borders_ro = ro_batch_new(4, r_texture_new_white_pixel());

    // black borders
    for (int i = 0; i < 4; i++) {
        self->L.borders_ro.rects[i].color = R_COLOR_BLACK;
    }

    // border poses
    {
        float l = tilemap_border_left(self->tilemap);
        float r = tilemap_border_right(self->tilemap);
        float t = tilemap_border_top(self->tilemap);
        float b = tilemap_border_bottom(self->tilemap);
        float w = r-l;
        float h = t-b;
        self->L.borders_ro.rects[0].pose = u_pose_new_aa(
                l - 1024, t + 1024, 1024, h + 2048);
        self->L.borders_ro.rects[1].pose = u_pose_new_aa(
                l - 1024, t + 1024, w + 2048, 1024);
        self->L.borders_ro.rects[2].pose = u_pose_new_aa(
                r, t + 1024, 1024, h + 2048);
        self->L.borders_ro.rects[3].pose = u_pose_new_aa(
                l - 1024, b, w + 2048, 1024);

    }
    ro_batch_update(&self->L.borders_ro);


    // test
    /*
    uImage img;
    rTexture tex_main, tex_refract;
    img = u_image_new_file(2, "res/ice_block.png");
    assume(u_image_valid(img), "wtf");
    tex_main = r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, 0));
    tex_refract = r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, 1));
    u_image_kill(&img);
    self->L.ice = ro_batchrefract_new(1, camera.gl_scale, tex_main, tex_refract);
    L.ice.view_aabb = camera.gl_view_aabb;
    for(int i=0; i<L.ice.num; i++) {
        self->L.ice.rects[i].pose = u_pose_new(260+16*i, 100, 32, 64);
        self->L.ice.rects[i].color.a=0.8;
    }
    ro_batchrefract_update(&self->L.ice);


    img = u_image_new_file(2, "res/mirror.png");
    assume(u_image_valid(img), "wtf");
    tex_main = r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, 0));
    tex_refract = r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, 1));
    u_image_kill(&img);
    self->L.mirror = ro_batchrefract_new(1, camera.gl_scale, tex_main, tex_refract);
    L.mirror.view_aabb = camera.gl_view_aabb;
    for(int i=0; i<self->L.mirror.num; i++) {
        self->L.mirror.rects[i].pose = u_pose_new(120+32*i, 100, 32, 64);
    }
    ro_batchrefract_update(&self->L.mirror);
    */
    
    return self;
}

void level_kill(Level **self_ptr) {
    Level *self = *self_ptr;
    if(!self)
        return;
        
    background_kill(&self->background);
    tilemap_kill();
    goal_kill();
    carrot_kill();
    butterfly_kill();
    flag_kill();
    for(int i=0; i<L.bubbles_size; i++) {
        speechbubble_kill(&self->L.bubbles[i]);
    }
    dead_kill();
    controller_kill();
    hud_kill();
    scripts_kill();
    
    unload_game();

    ro_batch_kill(&self->L.borders_ro);

    // test
    /*
    ro_batchrefract_kill(&L.ice);
    ro_batchrefract_kill(&L.mirror);
    */
    
    rhc_free(self);
    *self_ptr = NULL;
}

void level_update(Level *self, float dtime) {  
    goal_update(dtime);
    dead_update(dtime);
    hud_update(dtime);
    if (!dead_is_dead()) {
        
        // module linkage
        scripts_update(dtime);
        
        background_update(self->background, dtime);
        tilemap_update(dtime);
        carrot_update(dtime);
        flag_update(dtime);
        for(int i=0; i<self->L.bubbles_size; i++) {
            speechbubble_update(&self->L.bubbles[i], dtime, hare.pos);
        }
        enemies_update(dtime);
        
        airstroke_update(dtime);
        butterfly_update(dtime);
        pixelparticles_update(dtime);
        
    }
}

void level_render(Level *self, const Camera_s *cam, const mat4 *hudcam_mat) {
    const mat4 *cam_main_mat = &cam->matrices_main.vp;
    
    background_render(self->background, cam);
    flag_render();
    goal_render();
    tilemap_render_back();

    // test
    //ro_batchrefract_render(&L.ice, camera.gl_main);
    //ro_batchrefract_render(&L.mirror, camera.gl_main);

    for(int i=0; i<self->L.bubbles_size; i++) {
        speechbubble_render(&self->L.bubbles[i]);
    }
    carrot_render();
    pixelparticles_render();
    airstroke_render();
    //enemies_render();
    hare_render();
    butterfly_render();
    tilemap_render_front();
    hud_render();
    dead_render();

    ro_batch_render(&self->L.borders_ro, cam_main_mat);

    controller_render();
    
}

