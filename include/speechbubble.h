#ifndef JUMPHARE_SPEECHBUBBLE_H
#define JUMPHARE_SPEECHBUBBLE_H

#include "r/ro_text.h"

typedef struct {
    rRoText text;
    rRoBatch bubble;    
} SpeechBubble;

void speechbubble_init(SpeechBubble *self, vec2 center, const char *emojitext);

void speechbubble_kill(SpeechBubble *self);

void speechbubble_update(SpeechBubble *self, float dtime);

void speechbubble_render(SpeechBubble *self);

#endif //JUMPHARE_SPEECHBUBBLE_H
