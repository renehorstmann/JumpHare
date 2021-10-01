#include "u/image.h"
#include "rhc/error.h"
#include "tiles.h"



//
// public
//

Tiles *tiles_new() {
    Tiles *self = rhc_calloc(sizeof *self);
    
    int tile_id = 1;
    self->size = 0;
    for (;;) {
        char file[128];
        sprintf(file, "res/tiles/tile_%02i.png", tile_id);

        uImage img = u_image_new_file(2, file);
        if (!u_image_valid(img))
            break;

        assume(img.cols == TILES_COLS * TILES_SIZE
               && img.rows == TILES_ROWS * TILES_SIZE,
               "wrong tiles size");

        rTexture tex = r_texture_new(img.cols, img.rows, TILES_COLS, TILES_ROWS, u_image_layer(img, 0));

        self->imgs[self->size] = img;
        self->textures[self->size] = tex;
        self->ids[self->size] = tile_id;

        tile_id++;
        self->size++;
    }
    log_info("tiles: loaded: %i", self->size);
    if (self->size == 0)
        log_error("tiles: failed! 0 tiles loaded! Put some into tiles/tile_xx.png, starting with xx=01");
        
    return self;
}

uColor_s tiles_pixel(const Tiles *self, uColor_s code, int pixel_c, int pixel_r, int layer) {
    int tile_id = code.b;
    int tile = code.a;

    if (tile_id == 0)
        return U_COLOR_TRANSPARENT;

    uImage img = self->imgs[tile_id - 1];

    int tile_col = tile % TILES_COLS;
    int tile_row = tile / TILES_COLS;

    return *u_image_pixel(img,
                        pixel_c + tile_col * TILES_SIZE,
                        pixel_r + tile_row * TILES_SIZE,
                        layer);
}

enum tiles_pixel_state tiles_get_state(const Tiles *self, uColor_s id) {
    if (u_color_equals(id, U_COLOR_TRANSPARENT))
        return TILES_PIXEL_EMPTY;

    // todo: use...
    int qsum = ucvec4_sum(id);

    if (id.r > 150 && id.g < 100)
        return TILES_PIXEL_KILL;
        
    if (id.b > 150 && id.g < 100)
        return TILES_PIXEL_ONEWAY_UP;

    if (id.r > 200 && id.g > 200)
        return TILES_PIXEL_SOLID_CLEAN;

    return TILES_PIXEL_SOLID_DIRTY;
}
