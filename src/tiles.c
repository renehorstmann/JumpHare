#include "r/texture.h"
#include "utilc/assume.h"
#include "io.h"
#include "tiles.h"

struct TilesGlobals_s tiles;

void tiles_init() {
	int tile_id = 1;
	tiles.size = 0;
    for(;;) {
    	char file[128];
    	sprintf(file, "res/tiles/tile_%02i.png", tile_id);
    	
	    Image *img = io_load_image(file, 2);
	    if(!img)
	        break;
	        
	    assume(img->cols == TILES_COLS * TILES_SIZE 
	    && img->rows == TILES_ROWS * TILES_SIZE,
	    "wrong tiles size");
	    
	    GLuint tex = r_texture_init(img->cols, img->rows, image_layer(img, 0));
	    
	    tiles.imgs[tiles.size] = img;
	    tiles.textures[tiles.size] = tex;
	    tiles.ids[tiles.size] = tile_id;
	    
	    tile_id++;
	    tiles.size++;
    }
    SDL_Log("tiles loaded: %i", tiles.size);
    if(tiles.size == 0)
        SDL_Log("WARNING: 0 tiles loaded! Put some into tiles/tile_xx.png, starting with xx=01");
}

Color_s tiles_pixel(Color_s code, int layer, int pixel_c, int pixel_r) {
	int tile_id = code.b;
	int tile = code.a;
	
	if(tile_id == 0)
	    return COLOR_TRANSPARENT;
	
	Image *img = tiles.imgs[tile_id-1];
	
	int tile_col = tile % TILES_COLS;
	int tile_row = tile / TILES_COLS;
	
	return *image_pixel(img, layer, 
	    pixel_c + tile_col * TILES_SIZE,
	    pixel_r + tile_row * TILES_SIZE);
}

enum tiles_pixel_state tiles_get_state(Color_s id) {
    if(color_equals(id, COLOR_TRANSPARENT))
        return TILES_PIXEL_EMPTY;

    if(id.r > 150)
        return TILES_PIXEL_KILL;

    return TILES_PIXEL_SOLID;
}
