#include "r/ro_batch.h"
#include "r/ro_batchrefract.h"
#include "r/texture.h"
#include "u/pose.h"
#include "u/image.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "rhc/error.h"
#include "scripts.h"
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

static void add_enemies(Level *self) {
    vec2 hedgehogs[64];
    int hedgehogs_num = tilemap_get_positions(self->tilemap, hedgehogs, 64, ENEMY_HEDGHEHOG_CODE, CODE_LAYER);
    
    enemies_add_hedgehogs(self->game.enemies, hedgehogs, hedgehogs_num);
}

static void load_game(Level *self) {
    
    vec2 start_pos;
    assume(tilemap_get_positions(self->tilemap, &start_pos, 1, START_CODE, CODE_LAYER) == 1, "start not found");
    
    vec2 flag_pos = self->flag->RO.active_pos;
    if(!sca_isnan(flag_pos.x))
        start_pos = flag_pos;
    
    self->game.hare = hare_new(start_pos.x, start_pos.y, 
    self->collision,
    self->pixelparticles,
    self->window_ref);

    self->game.enemies = enemies_new(self->collision, self->game.hare);
    add_enemies(self);
    
    self->game.airstroke = airstroke_new();
    
    self->game.camctrl = cameractrl_new();
}

static void unload_game(Level *self) {
    enemies_kill(&self->game.enemies);
    hare_kill(&self->game.hare);
    airstroke_kill(&self->game.airstroke);
    cameractrl_kill(&self->game.camctrl);
}

static void reset(Level *self) {
    unload_game(self);
    load_game(self);
    
    carrot_load(self->carrot);
    butterfly_load(self->butterfly);
}

static void dead_callback(void *ud) {
    Level *self = ud;
    reset(self);
}



//
// public
//

Level *level_new(int lvl, Camera_s *cam, const HudCamera_s *hudcam, const Tiles *tiles, eWindow *window, eInput *input, rRender *render) {
    Level *self = rhc_calloc(sizeof *self);
    
    self->window_ref = window;
    self->camera_ref = cam;
    
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
    
    self->pixelparticles = pixelparticles_new();        
    
    vec2 goal_pos;
    assume(tilemap_get_positions(self->tilemap, &goal_pos, 1, GOAL_CODE, CODE_LAYER) == 1, "level needs 1 goal");
    self->goal = goal_new(self->pixelparticles, goal_pos);

    vec2 carrot_pos[3];
    assume(tilemap_get_positions(self->tilemap, carrot_pos, 3, CARROT_CODE, CODE_LAYER) == 3, "level needs 3 carrots");
    self->carrot = carrot_new(self->pixelparticles, carrot_pos);
    
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

    self->dead = dead_new(dead_callback, self);
    self->controller = controller_new(input, cam, hudcam);
    self->hud = hud_new();
    self->collision = collision_new(self->tilemap);
    

    load_game(self);
    
    // level 1?
    hare_set_sleep(self->game.hare, true);

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

    
    return self;
}

void level_kill(Level **self_ptr) {
    Level *self = *self_ptr;
    if(!self)
        return;
        
    background_kill(&self->background);
    tilemap_kill(&self->tilemap);
    pixelparticles_kill(&self->pixelparticles);
    goal_kill(&self->goal);
    carrot_kill(&self->carrot);
    butterfly_kill(&self->butterfly);
    flag_kill(&self->flag);
    for(int i=0; i<self->L.bubbles_size; i++) {
        speechbubble_kill(&self->L.bubbles[i]);
    }
    dead_kill(&self->dead);
    controller_kill(&self->controller);
    hud_kill(&self->hud);
    
    
    unload_game(self);

    ro_batch_kill(&self->L.borders_ro);
    
    rhc_free(self);
    *self_ptr = NULL;
}

void level_update(Level *self, float dtime) {  
    goal_update(self->goal, dtime);
    dead_update(self->dead, dtime);
    hud_update(self->hud, 
    self->camera_ref, self->carrot, self->butterfly,
    dtime);
    
    if (!dead_is_dead(self->dead)) {
        
        // module linkage
        scripts_update(self, dtime);
        
        background_update(self->background, dtime);
        tilemap_update(self->tilemap, dtime);
        carrot_update(self->carrot, dtime);
        flag_update(self->flag, self->game.hare, dtime);
        for(int i=0; i<self->L.bubbles_size; i++) {
            speechbubble_update(&self->L.bubbles[i], dtime, self->game.hare->pos);
        }
        enemies_update(self->game.enemies, dtime);
        
        airstroke_update(self->game.airstroke, self->tilemap, dtime);
        butterfly_update(self->butterfly, dtime);
        pixelparticles_update(self->pixelparticles, dtime);
        
    }
}

void level_render(const Level *self, const Camera_s *cam, const mat4 *hudcam_mat) {
    const mat4 *cam_main_mat = &cam->matrices_main.vp;
    
    background_render(self->background, cam);
    flag_render(self->flag, cam_main_mat);
    goal_render(self->goal, cam_main_mat);
    tilemap_render_back(self->tilemap, cam_main_mat);


    for(int i=0; i<self->L.bubbles_size; i++) {
        speechbubble_render(&self->L.bubbles[i], cam_main_mat);
    }
    carrot_render(self->carrot, cam_main_mat);
    pixelparticles_render(self->pixelparticles, cam_main_mat);
    airstroke_render(self->game.airstroke, cam_main_mat);
    //enemies_render();
    hare_render(self->game.hare, cam_main_mat);
    butterfly_render(self->butterfly, cam_main_mat);
    tilemap_render_front(self->tilemap, cam_main_mat);
    hud_render(self->hud, hudcam_mat);
    dead_render(self->dead, cam_main_mat, hudcam_mat);

    ro_batch_render(&self->L.borders_ro, cam_main_mat, false);

    controller_render(self->controller, hudcam_mat);
    
}

