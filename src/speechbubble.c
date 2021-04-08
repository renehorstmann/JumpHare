#include <string.h>
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "camera.h"
#include "speechbubble.h"


static bool emojifont_uv_cb(mat4 *uv, char c) {
    static const int cols = 16*6;
    static const int rows = 16*6;
    static const int size_x = 16;
    static const int size_y = 16;

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

    float w = (float) size_x / cols;
    float h = (float) size_y / rows;

    *uv = u_pose_new(col * w, row * h, w, h);

    return nl;
}

void emojifont_init(rRoText *self, int max, const float *vp) {
    r_ro_text_init(self, max, emojifont_uv_cb, vp, r_texture_new_file("res/emojifont.png", NULL));
    self->size = (vec2) {16, 16};
    self->offset = (vec2) {18, 18};
}



void speechbubble_init(SpeechBubble *self, vec2 center, const char *emojitext) {
    int len = strlen(emojitext);
    emojifont_init(&self->text, len, camera.gl_main);
    vec2 size = r_ro_text_set_text(&self->text, emojitext);
    
    vec2 text_pos = vec2_sub_vec(center, vec2_scale(size, 0.5));
    u_pose_set_xy(&self->text.pose, text_pos.x, text_pos.y);
}

void speechbubble_kill(SpeechBubble *self) {
    r_ro_text_kill(&self->text);
    //r_ro_batch_kill(&self->bubble);
}

void speechbubble_update(SpeechBubble *self, float dtime) {
    
}

void speechbubble_render(SpeechBubble *self) {
    //r_ro_batch_render(&self->bubble);
    r_ro_text_render(&self->text);
}