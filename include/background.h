#ifndef JUMPHARE_BACKGROUND_H
#define JUMPHARE_BACKGROUND_H

#include <stdbool.h>

void background_init(float level_width, float level_height, 
        bool repeat_h, bool repeat_v, 
        const char *file);

void background_kill();

void background_update(float dtime);

void background_render();

#endif //JUMPHARE_BACKGROUND_H
