#include "mathc/float.h"
#include "r/ro_single.h"
#include "r/ro_text.h"
#include "r/texture.h"
#include "u/pose.h"
#include "camera.h"
#include "hud_camera.h"
#include "tilemap.h"
#include "hare.h"

#define MIN_SPEED_X 20
#define MAX_SPEED_X 100
#define MAX_SPEED_Y 200
#define JUMP_SPEED 200
#define DOUBLE_JUMP_SPEED_Y 175
#define DOUBLE_JUMP_SPEED_X_FACTOR 0.5
#define ACC 160
#define DEACC 240
#define GRAVITY -400

#define JUMP_START_TIME 0.1
#define DOUBLE_JUMP_START_TIME 0.25

#define COLLISION_DISTANCE 10

#define ANIMATION_GROUNDED_FPS 6

#define SET_JUMP_TIME -0.05

static struct {
	rRoSingle ro;
	rRoText input_text;
	
	enum hare_state state, prev_state;
	
	vec2 pos;
	vec2 speed;
	
	float jump_time;
	
	float set_speed_x;
	float set_jump_time;
} L;


static vec2 apply_speed(float dtime) {
	vec2 pos = L.pos;
	
	float set_speed_x = L.state == HARE_DOUBLE_JUMP?             L.set_speed_x * DOUBLE_JUMP_SPEED_X_FACTOR 
	        : L.set_speed_x;
	// x
	float diff = set_speed_x - L.speed.x;
    if(diff>0) {
    	L.speed.x = sca_min(L.speed.x
    	        + (L.speed.x < 0 ? DEACC : ACC)
    	        * dtime, set_speed_x);
    } else if(diff<0) {
    	L.speed.x = sca_max(L.speed.x 
    	        - (L.speed.x > 0 ? DEACC : ACC)
    	        * dtime, set_speed_x);
        }
    float actual_speed = sca_abs(L.speed.x) < MIN_SPEED_X? 0 : L.speed.x;
    
    
    pos.x += actual_speed * dtime;
    
    // y
    if(L.state == HARE_GROUNDED)
        return pos;
        
    L.speed.y += GRAVITY * dtime;
	L.speed.y = sca_clamp(L.speed.y, -MAX_SPEED_Y, MAX_SPEED_Y);
	pos.y += L.speed.y * dtime;
	
	return pos;
}

static void check_collision_grounded() {
	float a, b;

	a = tilemap_wall_left(L.pos.x, L.pos.y+4);
	
	if(L.pos.x < a+7) {
		// collision?
		L.pos.x = a + 7;
	}
	
	
	a = tilemap_wall_right(L.pos.x, L.pos.y+4);
	
	if(L.pos.x > a-7) {
		// collision?
		L.pos.x = a - 7;
	}
	
	
	a = tilemap_ground(L.pos.x-4, L.pos.y);
	b = tilemap_ground(L.pos.x+4, L.pos.y);
	
	if(L.pos.y <= a+20 || L.pos.y <= b+20) {
		L.pos.y = sca_max(a, b) + 14;
	} else {
		L.state = HARE_FALLING;
		L.speed.y = 0;
	}
}

static void check_collision_jumping() {
	float a, b;

	a = tilemap_wall_left(L.pos.x, L.pos.y+4);
	
	if(L.pos.x < a+7) {
		// collision?
		L.pos.x = a + 7;
	}
	
	
	a = tilemap_wall_right(L.pos.x, L.pos.y+4);
	
	if(L.pos.x > a-7) {
		// collision?
		L.pos.x = a - 7;
	}
}


static void check_collision_falling() {
	float a, b;
	
	a = tilemap_ceiling(L.pos.x-4, L.pos.y);
	b = tilemap_ceiling(L.pos.x+4, L.pos.y);
	
	if(L.pos.y > a-8 || L.pos.y > b-8) {
		// jump collision?
		L.pos.y = sca_min(a, b) - 8;
	}

	a = tilemap_wall_left(L.pos.x, L.pos.y-10);
	b = tilemap_wall_left(L.pos.x, L.pos.y+4);
	
	if(L.pos.x < a+7 || L.pos.x < b+7) {
		// collision?
		L.pos.x = sca_max(a, b) + 7;
	}
	
	
	a = tilemap_wall_right(L.pos.x, L.pos.y-10);
	b = tilemap_wall_right(L.pos.x, L.pos.y+4);
	
	if(L.pos.x > a-7 || L.pos.x > b-7) {
		// collision?
		L.pos.x = sca_min(a, b) - 7;
	}
	
	a = tilemap_ground(L.pos.x-4, L.pos.y);
	b = tilemap_ground(L.pos.x+4, L.pos.y);
	
	if(L.pos.y < a+19 || L.pos.y < b+19) {
		L.pos.y = sca_max(a, b) + 14;
		L.state = HARE_GROUNDED;
		L.speed.y = 0;
	}
	
}

static void animate(float dtime) {
	static float time = 0;
	static bool looking_left = false;
	
	int frame;
	if(L.state == HARE_GROUNDED) {
		float fps = ANIMATION_GROUNDED_FPS;
		int frames = 4;
		time = fmodf(time + dtime, frames / fps);
		frame = time * fps;
	} else {
		frame = 3;
		if(L.jump_time>=0) {
		    if(L.jump_time<0.1)
		        frame = 0;
		    else if(L.jump_time<0.3)
		        frame = 1;
		    else if(L.jump_time<0.5)
		        frame = 2;
		}
	}

	float w = 1.0 / 4.0;
	float h = 1.0 / 4.0;

	if (L.speed.x < -MIN_SPEED_X)
		looking_left = true;
	if (L.speed.x > MIN_SPEED_X)
		looking_left = false;

	float v;
	if(L.state == HARE_GROUNDED) {
	    v = sca_abs(L.speed.x) < MIN_SPEED_X? 0 : 1;
	    if (sca_abs(L.speed.x) >=MAX_SPEED_X)
		    v++;
	} else {
		v = 3;
	}
	
	if(!looking_left)
	    L.ro.rect.uv = u_pose_new(frame * w, v * h, w, h);
	else
	    L.ro.rect.uv = u_pose_new((1+frame) * w, v * h, -w, h);
}


void hare_init() {
	L.state = L.prev_state = HARE_FALLING;
	
	r_ro_single_init(&L.ro, camera.gl_main, r_texture_init_file("res/hare.png", NULL));
	
	u_pose_set_size(&L.ro.rect.pose, 32, 32);

        
    r_ro_text_init_font55(&L.input_text, 64, hud_camera.gl);
    for(int i=0; i<64; i++) {
    	L.input_text.ro.rects[i].color = (vec4) {{0, 0, 0, 1}};
    }
}

void hare_update(float dtime) {
	L.prev_state = L.state;
    vec2 prev_pos = L.pos;
    
    // check jumping
    if(L.set_jump_time <= 0) {
    	if(L.state == HARE_GROUNDED) {
    	    L.state = HARE_JUMPING;
    	    L.jump_time = 0;
    	    L.set_jump_time = 1;
        }
        
        if(L.state == HARE_FALLING && L.jump_time > DOUBLE_JUMP_START_TIME) {
        	L.speed.y = sca_max(L.speed.y, DOUBLE_JUMP_SPEED_Y);
        	L.state = HARE_DOUBLE_JUMP;
        }
        L.set_jump_time+=dtime;
    }
    
    L.jump_time += dtime;
    
    if(L.state == HARE_JUMPING) {
        if(L.jump_time>=JUMP_START_TIME && L.speed.y <= 0) {
        	L.speed.y = JUMP_SPEED;
        }
        if(L.jump_time>=0.5) {
        	L.state = HARE_FALLING;
        }
    }
    
    
    vec2 dst_pos = apply_speed(dtime);
    
    // collision checks
    float dist = vec2_distance(L.pos, dst_pos);
    int checks = ceilf(dist / COLLISION_DISTANCE);
    for(int i=1; i<=checks; i++) {
    	L.pos = vec2_mix(prev_pos, dst_pos, (float) i / checks);
    	
    	switch(L.state) {
    	case HARE_GROUNDED:
    	    check_collision_grounded();
    	    break;
    	case HARE_FALLING:
    	case HARE_DOUBLE_JUMP:
    	    check_collision_falling();
    	    break;
    	case HARE_JUMPING:
    	    check_collision_jumping();
    	    break;
    	default:
    	    assert(0 && "invalid hare state");
        }
    }
    
    u_pose_set_xy(&L.ro.rect.pose, L.pos.x, L.pos.y);	
	
	animate(dtime);
	
	// debug
	char text[64];
    sprintf(text, "%i %+08.3f %+05.3f\n  %+08.3f %+08.3f", 
            L.state, 
            L.set_speed_x, L.set_jump_time, 
            L.speed.x, L.speed.y);
	r_ro_text_set_text(&L.input_text, text);
    u_pose_set_xy(&L.input_text.pose, hud_camera_left(), hud_camera_top());
}

void hare_render() {
	r_ro_single_render(&L.ro);
	r_ro_text_render(&L.input_text);
}

enum hare_state hare_state() {
	return L.state;
}

vec2 hare_position() {
	return (vec2) {{L.pos.x, L.pos.y}};
}

// [-1 : 1]
void hare_set_speed(float dx) {
	L.set_speed_x = sca_clamp(dx, -1, 1) * MAX_SPEED_X;
}

void hare_jump() {
	L.set_jump_time = SET_JUMP_TIME;
}
