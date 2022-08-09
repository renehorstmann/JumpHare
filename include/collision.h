#ifndef JUMPHARE_COLLISION_H
#define JUMPHARE_COLLISION_H

#include "s/s.h"
#include "m/types/float.h"


enum collision_state {
  COLLISION_BOTTOM,
  COLLISION_TOP,
  COLLISION_LEFT,
  COLLISION_RIGHT,
  COLLISION_FALLING,
  COLLISION_KILL
};


typedef void (*collision_tilemap_cb_fn)(vec2 delta, enum collision_state state, void *user_data);

typedef struct {
    collision_tilemap_cb_fn cb;
    void *cb_user_data;
} CollisionCallback_s;


void collision_init();

void collision_kill();

void collision_tilemap_grounded(CollisionCallback_s callback, vec2 center, vec2 radius, vec2 speed);

void collision_tilemap_falling(CollisionCallback_s callback, vec2 center, vec2 radius, vec2 speed);


#endif //JUMPHARE_COLLISION_H
