#ifndef JUMPHARE_ENEMIES_H
#define JUMPHARE_ENEMIES_H

#include "r/ro_types.h"
#include "collision.h"

#define ENEMIES_MAX_TYPES 32


struct Enemies_Type;
struct Enemies;

typedef void (*update_fn)(struct Enemies_Type *self, float dtime);

struct Enemies_Type {
    const struct Enemies *enemies_ref;
    RoBatch ro;
    update_fn update;
    float time;
};

typedef struct Enemies {
    const Collision *collision_ref;
    struct {
        struct Enemies_Type types[ENEMIES_MAX_TYPES];
        int types_num;
    } L;
} Enemies;


Enemies *enemies_new(const Collision *collision);

void enemies_kill(Enemies **self_ptr);

void enemies_update(Enemies *self, float dtime);

void enemies_render(Enemies *self, const mat4 *cam_mat);

void enemies_add_hedgehogs(Enemies *self, const vec2 *positions, int n);

#endif //JUMPHARE_ENEMIES_H
