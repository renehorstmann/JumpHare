#include "r/ro_single.h"
#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "utilc/assume.h"
#include "camera.h"
#include "io.h"
#include "background.h"

#define BACKGROUND_COLS 256
#define BACKGROUND_ROWS 128
#define MAX_LEVEL_SIZE 4096

static struct {
    rRoBatch ro[CAMERA_BACKGROUNDS];
} L;

static void set_image(const char *file) {
    Image *img = io_load_image(file, CAMERA_BACKGROUNDS);
    assume(img->cols == BACKGROUND_COLS && img->rows == BACKGROUND_ROWS, "wrong background format");

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++) {
        r_texture_update(L.ro[i].tex, img->cols, img->rows, image_layer(img, i));
    }
}


void background_init(const char *file) {
    int size = (int) ceilf((float) MAX_LEVEL_SIZE / BACKGROUND_COLS);

    float w = 512;
    float h = 256;


    for (int i = 0; i < CAMERA_BACKGROUNDS; i++) {
        GLuint tex = r_texture_init(BACKGROUND_COLS, BACKGROUND_ROWS, NULL);
        r_ro_batch_init(&L.ro[i], size, camera.gl_background[i], tex);

        for (int j = 0; j < size; j++) {
            L.ro[i].rects[j].pose = u_pose_new(w * j, 0, w, h);
        }
        r_ro_batch_update(&L.ro[i]);
    }
    set_image("res/backgrounds/greenhills.png");
}


void background_kill() {
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++) {
        r_ro_batch_kill(&L.ro[i]);
    }
    memset(&L, 0, sizeof(L));
}


void background_update(float dtime) {

}

void background_render() {
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        r_ro_batch_render(&L.ro[i]);
}

