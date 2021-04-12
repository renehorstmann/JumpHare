#ifndef JUMPHARE_SPEECHBUBBLE_H
#define JUMPHARE_SPEECHBUBBLE_H

#include "r/ro_text.h"

typedef struct {
    RoText text;
    RoBatch bubble;
    vec2 position;    // reset to NAN, to ignore alpha blend (or just dont call update)
} SpeechBubble;

void speechbubble_init(SpeechBubble *self, vec2 position, const char *emojitext);

void speechbubble_kill(SpeechBubble *self);

void speechbubble_update(SpeechBubble *self, float dtime);

void speechbubble_render(SpeechBubble *self);

#endif //JUMPHARE_SPEECHBUBBLE_H
