#ifndef JUMPHARE_BUTTON_H
#define JUMPHARE_BUTTON_H

#include <stdbool.h>
#include "e/input.h"
#include "r/rect.h"

void button_init_uv(rRect_s *self);

bool button_is_pressed(rRect_s *self);

void button_set_pressed(rRect_s *self, bool pressed);

bool button_clicked(rRect_s *self, ePointer_s pointer);

bool button_pressed(rRect_s *self, ePointer_s pointer);

bool button_toggled(rRect_s *self, ePointer_s pointer);

#endif //JUMPHARE_BUTTON_H
