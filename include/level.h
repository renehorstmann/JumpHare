#ifndef JUMPHARE_LEVEL_H
#define JUMPHARE_LEVEL_H

#include "e/window.h"
#include "e/input.h"
#include "r/ro_types.h"
#include "r/render.h"

#include "camera.h"
#include "cameractrl.h"
#include "background.h"
#include "speechbubble.h"
#include "tilemap.h"
#include "goal.h"
#include "carrot.h"
#include "butterfly.h"
#include "flag.h"
#include "dead.h"
#include "controller.h"
#include "hud.h"
#include "collision.h"
#include "enemies.h"
#include "hare.h"
#include "airstroke.h"
#include "pixelparticles.h"

typedef struct {
    eWindow *window_ref;
    Camera_s *camera_ref;
    
    Background *background;
    Tilemap *tilemap;
    PixelParticles *pixelparticles;
    Goal *goal;
    Carrot *carrot;
    Butterfly *butterfly;
    Flag *flag;
    Dead *dead;
    Controller *controller;
    Hud *hud;
    Collision *collision;
    
    
    struct {
        Enemies *enemies;
        Hare *hare;
        Airstroke *airstroke;
        CameraControl_s *camctrl;
    } game;
    
    struct {
        RoBatch borders_ro;
        int current_lvl;
        SpeechBubble bubbles[3];
        int bubbles_size;
        int state;

        // test
        RoBatchRefract ice, mirror;
    } L;
} Level;

Level *level_new(int lvl, Camera_s *cam, const HudCamera_s *hudcam, const Tiles *tiles, eWindow *window, eInput *input, rRender *render);

void level_kill(Level **self_ptr);

void level_update(Level *self, float dtime);

void level_render(const Level *self, const Camera_s *cam, const mat4 *hudcam_mat);


#endif //JUMPHARE_LEVEL_H
