#ifndef JUMPHARE_COLLISION_H
#define JUMPHARE_COLLISION_H

#include <stdbool.h>
#include "tilemap.h"


typedef struct {
    const Tilemap *tilemap_ref;
} Collision;

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


Collision *collision_new(const Tilemap *tilemap);

void collision_kill(Collision **self_ptr);

void collision_tilemap_grounded(const Collision *self, CollisionCallback_s callback, vec2 center, vec2 radius, vec2 speed);

void collision_tilemap_falling(const Collision *self, CollisionCallback_s callback, vec2 center, vec2 radius, vec2 speed);


#endif //JUMPHARE_COLLISION_H
