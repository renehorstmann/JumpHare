
#include "r/texture.h"
#include "tiles.h"


struct TilesGlobals_s tiles;

void tiles_init() {
    int tile_id = 1;
    tiles.size = 0;
    for(;;) {
        char file[128];
        sprintf(file, "res/tiles/tile_%02i.png", tile_id);
        GLuint tex = r_texture_init_file(file, NULL);
        if(!tex)
            break;

        tiles.textures[tiles.size] = tex;
        tiles.ids[tiles.size] = tile_id;

        tile_id++;
        tiles.size++;
    }
    SDL_Log("tiles loaded: %i", tiles.size);
    if(tiles.size == 0)
        SDL_Log("WARNING: 0 tiles loaded! Put some into tiles/tile_xx.png, starting with xx=01");
}
