#ifndef JUMPHARE_SPEECHBUBBLE_H
#define JUMPHARE_SPEECHBUBBLE_H

#include "r/ro_text.h"

typedef struct {
    RoText text;
    RoBatch bubble;
    vec2 position;    // reset to NAN, to ignore alpha blend (or just dont call update)
} SpeechBubble;

SpeechBubble speechbubble_new(vec2 position, const char *emojitext);

void speechbubble_kill(SpeechBubble *self);

void speechbubble_update(SpeechBubble *self, float dtime, vec2 blend_pos);

void speechbubble_render(const SpeechBubble *self, const mat4 *cam_mat);

#endif //JUMPHARE_SPEECHBUBBLE_H
