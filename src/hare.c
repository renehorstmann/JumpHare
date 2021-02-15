#include "mathc/float.h"
#include "r/ro_single.h"
#include "r/texture.h"
#include "u/pose.h"
#include "camera.h"
#include "hare.h"


static struct {
    rRoSingle ro;
    float dx;
    float looking_left;

    rRoSingle bg;
} L;

void hare_init() {
    r_ro_single_init(&L.ro, camera.gl, r_texture_init_file("res/hare.png", NULL));

    r_ro_single_init(&L.bg, camera.gl, r_texture_init_file("res/grass.png", NULL));
}

void hare_update(float dtime) {
    static float x = 0;
    x += dtime * L.dx;
    if (x > camera_right())
        x = camera_left();
    if (x < camera_left())
        x = camera_right();
    float px = floorf(x);


    float fps = 6;
    int frames = 4;
    static float time = 0;
    time = fmodf(time + dtime, frames / fps);
    int frame = time * fps;

    float w = 1.0 / 4.0;
    float h = 1.0 / 2.0;

    if (L.dx < 0)
        L.looking_left = true;
    if (L.dx > 0)
        L.looking_left = false;

    if (L.looking_left)
        w = -w;

    float v = L.dx == 0 ? 0 : 1;

    u_pose_set(&L.ro.rect.pose, px, 0, 32, 32, 0);
    u_pose_set(&L.ro.rect.uv, frame * w, v * h, w, h, 0);

    u_pose_set(&L.bg.rect.pose, 0, -5, 128 * 2, 64, 0);
    u_pose_set_w(&L.bg.rect.uv, 2);
}

void hare_render() {
    r_ro_single_render(&L.bg);
    r_ro_single_render(&L.ro);
}


void hare_set_speed(float dx) {
    
    // [0 || += 10 : +-80]
    dx = sca_abs(dx) < 10 ? 0 : dx;
    L.dx = sca_sign(dx) * sca_min(sca_abs(dx), 80);
}

