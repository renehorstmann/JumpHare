#include "tilemap.h"
#include "tiles.h"
#include "collision.h"


#define SCAN_SIZE 4.0
#define SCANNER_DISTANCE 0.66


static bool scan_left(Collision_s self, float x, float y) {
    Color_s id;
    float pos = tilemap_wall_left(x + SCAN_SIZE, y, &id);
    if(pos <= x) 
        return false;
    
    enum tiles_pixel_state pixel = tiles_get_state(id);
    
    if(pixel == TILES_PIXEL_ONEWAY_UP)
        return false;
        
    enum collision_state state = pixel==TILES_PIXEL_KILL? COLLISION_KILL : COLLISION_LEFT;
    
    vec2 delta = {{pos - x, 0}};
    self.cb(delta, state, self.cb_user_data);
    return true;
}

static bool scan_right(Collision_s self, float x, float y) {
    Color_s id;
    float pos = tilemap_wall_right(x - SCAN_SIZE, y, &id);
    if(pos >= x) 
        return false;
    
    enum tiles_pixel_state pixel = tiles_get_state(id);
    
    if(pixel == TILES_PIXEL_ONEWAY_UP)
        return false;
        
    enum collision_state state = pixel==TILES_PIXEL_KILL? COLLISION_KILL : COLLISION_RIGHT;
    
    vec2 delta = {{pos - x, 0}};
    self.cb(delta, state, self.cb_user_data);
    return true;
}

static bool scan_top(Collision_s self, float x, float y) {
    Color_s id;
    float pos = tilemap_ceiling(x, y - SCAN_SIZE, &id);
    if(pos >= y) 
        return false;
    
    enum tiles_pixel_state pixel = tiles_get_state(id);
    
    if(pixel == TILES_PIXEL_ONEWAY_UP)
        return false;
        
    enum collision_state state = pixel==TILES_PIXEL_KILL? COLLISION_KILL : COLLISION_TOP;
    
    vec2 delta = {{0, pos - y}};
    self.cb(delta, state, self.cb_user_data);
    return true;
}

static bool scan_bottom_falling(Collision_s self, float x, float y, float speed_y) {
    Color_s id;
    float pos = tilemap_ground(x, y + SCAN_SIZE, &id);
    if(pos <= y) 
        return false;
    
    enum tiles_pixel_state pixel = tiles_get_state(id);
    
    enum collision_state state = pixel==TILES_PIXEL_KILL? COLLISION_KILL : COLLISION_BOTTOM;
    
    vec2 delta = {{0, pos - y}};
    self.cb(delta, state, self.cb_user_data);
    return true;
}

static void scan_bottom_grounded(Collision_s self, float x_a, float x_b, float y) {
    Color_s id_a, id_b;
    float pos_a = tilemap_ground(x_a, y + SCAN_SIZE, &id_a);
    float pos_b = tilemap_ground(x_b, y + SCAN_SIZE, &id_b);
    
    if(pos_a < y - SCAN_SIZE && pos_b < y - SCAN_SIZE) {
        self.cb((vec2){{0}}, COLLISION_FALLING, self.cb_user_data);
        return;
    }
    
    enum collision_state state;
    vec2 delta = {{0}};
    if(pos_a > pos_b) {
        enum tiles_pixel_state pixel = tiles_get_state(id_a);
        state = pixel == TILES_PIXEL_KILL? COLLISION_KILL : COLLISION_BOTTOM;
        delta.y = pos_a - y;
    } else {
        enum tiles_pixel_state pixel = tiles_get_state(id_b);
        state = pixel == TILES_PIXEL_KILL? COLLISION_KILL : COLLISION_BOTTOM;
        delta.y = pos_b - y;
    }
    
    self.cb(delta, state, self.cb_user_data);
}




void collision_tilemap_grounded(Collision_s self, vec2 center, vec2 radius, vec2 speed) {
    
    if(scan_left(self, center.x-radius.x, center.y))
        return;
        
    if(scan_right(self, center.x+radius.x, center.y))
        return;
    
    scan_bottom_grounded(self, 
            center.x-radius.x*SCANNER_DISTANCE,
            center.x+radius.x*SCANNER_DISTANCE,
            center.y - radius.y);
}

void collision_tilemap_falling(Collision_s self, vec2 center, vec2 radius, vec2 speed) {
    
    if(speed.y>0) {
        if(scan_top(self, 
                center.x-radius.x*SCANNER_DISTANCE,
                center.y + radius.y))
            return;
        
        if(scan_top(self, 
                center.x+radius.x*SCANNER_DISTANCE, 
                center.y + radius.y))
            return;
    }
    
    
    if(scan_left(self, center.x-radius.x, center.y - radius.y*SCANNER_DISTANCE))
        return;
        
    if(scan_left(self, center.x-radius.x, center.y + radius.y*SCANNER_DISTANCE))
        return;
        
        
    if(scan_right(self, center.x+radius.x, center.y - radius.y*SCANNER_DISTANCE))
        return;
        
    if(scan_right(self, center.x+radius.x, center.y + radius.y*SCANNER_DISTANCE))
        return;
       
       
    if(speed.y>0)
        return;
        
    if(scan_bottom_falling(self, center.x-radius.x*SCANNER_DISTANCE, center.y - radius.y, speed.y))
        return;
            
    if(scan_bottom_falling(self, center.x+radius.x*SCANNER_DISTANCE, center.y - radius.y, speed.y))
        return;
}
