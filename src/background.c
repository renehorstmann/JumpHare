#include "r/ro_single.h"
#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "utilc/assume.h"
#include "camera.h"
#include "io.h"
#include "background.h"

#define PIXEL_SIZE 2.0

static struct {
    rRoBatch ro[CAMERA_BACKGROUNDS];
} L;


void background_init(float level_width, float level_height, 
        bool repeat_h, bool repeat_v, 
        const char *file) {
    Image *img = io_load_image(file, CAMERA_BACKGROUNDS);
    
    float rows = img->rows * PIXEL_SIZE;
    float cols = img->cols * PIXEL_SIZE;

    int size_h = repeat_h? ceilf(level_width / cols) : 1;
    int size_v = repeat_v? ceilf(level_height / rows) : 1;

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++) {
        GLuint tex = r_texture_new(img->cols, img->rows, image_layer(img, i));
        r_ro_batch_init(&L.ro[i], size_h*size_v, camera.gl_background[i], tex);

        for (int v = 0; v < size_v; v++) {
            for(int h = 0; h < size_h; h++) {
                L.ro[i].rects[v*size_h+h].pose = 
                        u_pose_new_aa(
                        cols * h, rows *(v+1), 
                        cols, rows);
            }
        }
        r_ro_batch_update(&L.ro[i]);
    }
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

