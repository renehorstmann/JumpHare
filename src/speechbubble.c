#include <string.h>
#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "speechbubble.h"


#define OFFSET_Y 64
#define MAX_DIST 60.0
#define MIN_DIST 25.0

#define COLUMNS 6
#define ROWS 6


//
// private
//

static bool emojifont_sprite_cb(vec2 *sprite, char c) {

    bool nl = false;
    if (c == '\n') {
        nl = true;
        c = ' ';
    }
    
    // default, space = 0, 0
    int row = 0; 
    int col = 0;
    
    if(c=='N') {col=1;} // Neutral
    if(c=='H') {col=2;} // Happy
    if(c=='O') {col=3;} // O face
    if(c=='S') {col=4;} // Sad
    if(c=='T') {col=5;} // Tired
    
    if(c=='F') {row=1;}        // Feets
    if(c=='K') {row=1; col=1;} // Kill
    if(c=='G') {row=1; col=2;} // Goal
    if(c=='f') {row=1; col=3;} // flag
    if(c=='B') {row=1; col=4;} // Butterfly
    
    if(c=='L') {row=2;}        // Love
    if(c=='s') {row=2; col=1;} // shootingStar
    if(c=='C') {row=2; col=2;} // Carrot
    if(c=='E') {row=2; col=3;} // Eaten carrot
    if(c=='a') {row=2; col=4;} // airstrike
    if(c=='Z') {row=2; col=5;} // Zzz
    
    if(c=='=') {row=3;}        // =
    if(c=='>') {row=3; col=2;} // arrow right
    if(c=='<') {row=3; col=3;} // arrow left
    if(c=='t') {row=3; col=4;} // top arrow
    if(c=='b') {row=3; col=5;} // bottom arrow

    sprite->x = col;
    sprite->y = row;

    return nl;
}



//
// public
//


// public ?
static RoText emojifont_new(int max) {
    RoText self = ro_text_new(max, emojifont_sprite_cb, r_texture_new_file(COLUMNS, ROWS, "res/emojifont.png"));
    self.size = (vec2) {16, 16};
    self.offset = (vec2) {18, 18};
    return self;
}



SpeechBubble speechbubble_new(vec2 position, const char *emojitext) {
    SpeechBubble bubble = {0};
    SpeechBubble *self = &bubble;
    
    self->position = position;
    
    int len = strlen(emojitext);
    self->text = emojifont_new(len);
    vec2 size = ro_text_set_text(&self->text, emojitext);
    
    float text_x = position.x - size.x/2;
    float text_y = position.y + OFFSET_Y + size.y/2;
    u_pose_set_xy(&self->text.pose, text_x, text_y);
    
    int rows = floorf(size.y/16)+2;
    int cols = floorf(size.x/16)+2;
    
    float bubbly_x = text_x-18/2;
    float bubble_y = text_y+18/2;
    
    self->bubble = ro_batch_new(rows*cols, r_texture_new_file(1, 1, "res/speechbubble.png"));
    
    for(int r=0; r<rows; r++) {
        for(int c=0; c<cols;c++) {
            float u, v;
            if(r==0)
                v = 0;
            else if(r==rows-1)
                v = 2.0/3.0;
            else 
                v = 1.0/3.0;
                
            if(c==0)
                u = 0;
            else if(c==cols-1)
                u = 2.0/3.0;
            else
                u = 1.0/3.0;
                
            self->bubble.rects[r*cols+c].uv = u_pose_new(u, v, 1.0/3.0, 1.0/3.0);
            
            self->bubble.rects[r*cols+c].pose = u_pose_new(
                    bubbly_x + c*18,
                    bubble_y - r*18,
                    18, 18);
        }
    }
    
    ro_batch_update(&self->bubble);
    
    return bubble;
}

void speechbubble_kill(SpeechBubble *self) {
    ro_text_kill(&self->text);
    ro_batch_kill(&self->bubble);
}

void speechbubble_update(SpeechBubble *self, float dtime, vec2 blend_pos) {
    if(sca_isnan(self->position.x))
        return;
    
    float dist = vec2_distance(blend_pos, self->position);
    
    float alpha;
    if(dist < MIN_DIST) {
        alpha = 1;
    } else {
        float t = dist / MAX_DIST;
        alpha = sca_clamp(sca_mix(1, 0, t), 0, 1);
    }
    
    for(int i=0;i<self->text.ro.num;i++) {
        self->text.ro.rects[i].color.a = alpha;
    }
    for(int i=0;i<self->bubble.num;i++) {
        self->bubble.rects[i].color.a = alpha;
    }
    
    
    ro_batch_update(&self->text.ro);
    ro_batch_update(&self->bubble);
}

void speechbubble_render(const SpeechBubble *self, const mat4 *cam_mat) {
    ro_batch_render(&self->bubble, cam_mat);
    ro_text_render(&self->text, cam_mat);
}
