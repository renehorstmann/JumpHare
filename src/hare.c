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
#define DOUBLE_JUMP_SPEED 200
#define ACC 160
#define DEACC 240
#define GRAVITY -400

#define SET_JUMP_TIME -0.05

static struct {
	rRoSingle ro;
	rRoSingle coll[8];
	rRoText input_text;
	
	enum hare_state state;
	
	vec2 pos;
	vec2 last_pos;
	
	vec2 speed;
	float looking_left;

	float jump_time;
	
	float set_speed_x;
	float set_jump_time;
} L;


static void apply_speed_x(float dtime) {
	float diff = L.set_speed_x - L.speed.x;
    if(diff>0) {
    	L.speed.x = sca_min(L.speed.x
    	        + (L.speed.x < 0 ? DEACC : ACC)
    	        * dtime, L.set_speed_x);
    } else if(diff<0) {
    	L.speed.x = sca_max(L.speed.x 
    	        - (L.speed.x > 0 ? DEACC : ACC)
    	        * dtime, L.set_speed_x);
        }
    float actual_speed = sca_abs(L.speed.x) < MIN_SPEED_X? 0 : L.speed.x;
    
    
    L.pos.x += actual_speed * dtime;
}

static void apply_speed_y(float dtime) {
	L.speed.y += GRAVITY * dtime;
	L.speed.y = sca_clamp(L.speed.y, -MAX_SPEED_Y, MAX_SPEED_Y);
	L.pos.y += L.speed.y * dtime;
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


static void check_collision_falling() {
	float a, b;
	
	a = tilemap_ceiling(L.pos.x-4, L.pos.y+4);
	b = tilemap_ceiling(L.pos.x+4, L.pos.y+4);
	L.coll[0].rect.pose=u_pose_new(L.pos.x-4, a, 3, 3);
	L.coll[1].rect.pose=u_pose_new(L.pos.x+4, b, 3, 3);
	
	if(L.pos.y > a-8 || L.pos.y > b-8) {
		// jump collision?
		L.pos.y = sca_min(a, b) - 8;
	}

	a = tilemap_wall_left(L.pos.x-4, L.pos.y-8);
	b = tilemap_wall_left(L.pos.x-4, L.pos.y+4);
	L.coll[2].rect.pose=u_pose_new(a, L.pos.y-8, 3, 3);
	L.coll[3].rect.pose=u_pose_new(b, L.pos.y+4, 3, 3);
	
	if(L.pos.x < a+7 || L.pos.x < b+7) {
		// collision?
		L.pos.x = sca_max(a, b) + 7;
	}
	
	
	a = tilemap_wall_right(L.pos.x+4, L.pos.y-8);
	b = tilemap_wall_right(L.pos.x+4, L.pos.y+4);
	L.coll[4].rect.pose=u_pose_new(a, L.pos.y-8, 3, 3);
	L.coll[5].rect.pose=u_pose_new(b, L.pos.y+4, 3, 3);
	
	if(L.pos.x > a-7 || L.pos.x > b-7) {
		// collision?
		L.pos.x = sca_min(a, b) - 7;
	}
	
	a = tilemap_ground(L.pos.x-4, L.pos.y-4);
	b = tilemap_ground(L.pos.x+4, L.pos.y-4);
	L.coll[6].rect.pose=u_pose_new(L.pos.x-4, a, 3, 3);
	L.coll[7].rect.pose=u_pose_new(L.pos.x+4, b, 3, 3);
	
	if(L.pos.y <= a+13 || L.pos.y <= b+13) {
		L.pos.y = sca_max(a, b) + 14;
		L.state = HARE_GROUNDED;
		L.speed.y = 0;
	}
	
}


void hare_init() {
	L.state = HARE_GROUNDED;
	L.jump_time = -1;
	
	r_ro_single_init(&L.ro, camera.gl_main, r_texture_init_file("res/hare.png", NULL));

    for(int i=0; i<8; i++)
        r_ro_single_init(&L.coll[i], camera.gl_main, 0);
        
    r_ro_text_init_font55(&L.input_text, 64, hud_camera.gl);
    for(int i=0; i<64; i++) {
    	L.input_text.ro.rects[i].color = (vec4) {{0, 0, 0, 1}};
    }
}

void hare_update(float dtime) {
    char text[64];
    sprintf(text, "%i %+08.3f %+05.3f\n ", L.state, L.set_speed_x, L.set_jump_time);
    
    if(L.set_jump_time <= 0) {
    	if(L.state == HARE_GROUNDED) {
    	    L.state = HARE_JUMPING;
    	    L.jump_time = 0;
    	    L.set_jump_time = 1;
        }
        L.set_jump_time+=dtime;
    }
    
    L.jump_time += dtime;
    
    if(L.state == HARE_JUMPING) {
        if(L.jump_time>=0.1) {
        	L.speed.y = JUMP_SPEED;
        	L.state = HARE_FALLING;
        }
    }
        
    apply_speed_x(dtime);
    if(L.state != HARE_GROUNDED)
        apply_speed_y(dtime);
    
    sprintf(text + strlen(text), " %+08.3f %+08.3f", 
                L.speed.x, L.speed.y);
    switch(L.state) {
    	case HARE_GROUNDED:
    	case HARE_JUMPING:
    	    check_collision_grounded();
    	    break;
    	case HARE_FALLING:
    	case HARE_DOUBLE_JUMP:
    	    check_collision_falling();
    	    break;
    	default:
    	    assert(0 && "invalid hare state");
    }
	
	
	
	int frame;
	if(L.state == HARE_GROUNDED) {
		float fps = 6;
		int frames = 4;
		static float time = 0;
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
		L.looking_left = true;
	if (L.speed.x > MIN_SPEED_X)
		L.looking_left = false;

	float v;
	if(L.state == HARE_GROUNDED) {
	    v = sca_abs(L.speed.x) < MIN_SPEED_X? 0 : 1;
	    if (sca_abs(L.speed.x) >=MAX_SPEED_X)
		    v++;
	} else {
		v = 3;
	}
		
    vec2 p = vec2_mix(L.pos, L.last_pos, 0.5);
	
	u_pose_set(&L.ro.rect.pose, p.x, p.y, 32, 32, 0);	
	if(!L.looking_left)
	    u_pose_set(&L.ro.rect.uv, frame * w, v * h, w, h, 0);
	else
	    u_pose_set(&L.ro.rect.uv, (1+frame) * w, v * h, -w, h, 0);
	
	
	L.last_pos = L.pos;
	
	r_ro_text_set_text(&L.input_text, text);
    u_pose_set_xy(&L.input_text.pose, hud_camera_left(), hud_camera_top());
}

void hare_render() {
	r_ro_single_render(&L.ro);
	//for(int i=0; i<8;i++)
	//    r_ro_single_render(&L.coll[i]);
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
