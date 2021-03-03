#include "mathc/float.h"
#include "r/ro_single.h"
#include "r/ro_text.h"
#include "r/texture.h"
#include "u/pose.h"
#include "camera.h"
#include "hud_camera.h"
#include "tilemap.h"
#include "hare.h"

#define MIN_SPEED_X 10
#define MAX_SPEED_X 80
#define MAX_SPEED_Y 160
#define ACC 2
#define DEACC 4
#define GRAVITY -5

static struct {
	rRoSingle ro;
	rRoSingle coll[8];
	rRoText input_text;
	
	vec2 pos;
	vec2 last_pos;
	
	vec2 speed;
	float looking_left;

	float jump_time;
	
	float set_speed_x;
	bool set_jump;
	bool grounded;
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
    float actual_speed_x = L.speed.x * MAX_SPEED_X;
    actual_speed_x = sca_abs(actual_speed_x) < MIN_SPEED_X? 0 : actual_speed_x;
    
    
    L.pos.x += actual_speed_x * dtime;
}

static void apply_speed_y(float dtime) {
	L.speed.y += GRAVITY * dtime;
	L.speed.y = sca_clamp(L.speed.y, -1, 1);
	float actual_speed = L.speed.y * MAX_SPEED_Y;
	L.pos.y += actual_speed * dtime;
}


static void check_collision() {
	float a, b;
	
	a = tilemap_ceiling(L.pos.x-4, L.pos.y+4);
	b = tilemap_ceiling(L.pos.x+4, L.pos.y+4);
	L.coll[0].rect.pose=u_pose_new(L.pos.x-4, a, 3, 3);
	L.coll[1].rect.pose=u_pose_new(L.pos.x+4, b, 3, 3);
	
	if(L.pos.y > a-8 || L.pos.y > b-8) {
		// jump collision?
		L.pos.y = sca_min(a, b) - 8;
	}

	a = tilemap_wall_left(L.pos.x-4, L.pos.y-4);
	b = tilemap_wall_left(L.pos.x-4, L.pos.y+4);
	L.coll[2].rect.pose=u_pose_new(a, L.pos.y-4, 3, 3);
	L.coll[3].rect.pose=u_pose_new(b, L.pos.y+4, 3, 3);
	
	if(L.pos.x < a+7 || L.pos.x < b+7) {
		// collision?
		L.pos.x = sca_max(a, b) + 7;
	}
	
	
	a = tilemap_wall_right(L.pos.x+4, L.pos.y-4);
	b = tilemap_wall_right(L.pos.x+4, L.pos.y+4);
	L.coll[4].rect.pose=u_pose_new(a, L.pos.y-4, 3, 3);
	L.coll[5].rect.pose=u_pose_new(b, L.pos.y+4, 3, 3);
	
	if(L.pos.x > a-7 || L.pos.x > b-7) {
		// collision?
		L.pos.x = sca_min(a, b) - 7;
	}
	
	a = tilemap_ground(L.pos.x-4, L.pos.y-4);
	b = tilemap_ground(L.pos.x+4, L.pos.y-4);
	L.coll[6].rect.pose=u_pose_new(L.pos.x-4, a, 3, 3);
	L.coll[7].rect.pose=u_pose_new(L.pos.x+4, b, 3, 3);
	
	if(L.pos.y < a+20 || L.pos.y < b+20) {
		L.grounded = true;
		L.pos.y = sca_max(a, b) + 14;
	} else {
		L.grounded = false;
	}
	
}


void hare_init() {
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
    sprintf(text, "%+6.3f %i", L.set_speed_x, L.set_jump);
    
    
    if(L.grounded && L.set_jump) {
    	// jump
    }
    L.set_jump = false;
    
    apply_speed_x(dtime);
    if(!L.grounded)
        apply_speed_y(dtime);
    
    sprintf(text + strlen(text), " %+6.3f %+6.3f", 
                L.speed.x, L.speed.y);
    
	check_collision();
	
	
	
	int frame;
	{
		float fps = 6;
		int frames = 4;
		static float time = 0;
		time = fmodf(time + dtime, frames / fps);
		frame = time * fps;
	}

	float w = 1.0 / 4.0;
	float h = 1.0 / 4.0;

	if (L.speed.x < 0)
		L.looking_left = true;
	if (L.speed.x > 0)
		L.looking_left = false;

	float v = L.speed.x == 0 ? 0 : 1;
	if (sca_abs(L.speed.x) > 0.8)
		v++;
		
		
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
	for(int i=0; i<8;i++)
	    r_ro_single_render(&L.coll[i]);
	r_ro_text_render(&L.input_text);
}

vec2 hare_position() {
	return (vec2) {{L.pos.x, L.pos.y}};
}

// [-1 : 1]
void hare_set_speed(float dx) {
	L.set_speed_x = sca_clamp(dx, -1, 1);
}

void hare_jump() {
	L.set_jump = true;
}
