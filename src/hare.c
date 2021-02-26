#include "mathc/float.h"
#include "r/ro_single.h"
#include "r/texture.h"
#include "u/pose.h"
#include "camera.h"
#include "tilemap.h"
#include "hare.h"

static struct {
	rRoSingle ro;
	
	float x, y;
	
	float dx, dy;
	float looking_left;

	float jump_time;
	float speed_y;
} L;

void hare_init() {
	L.jump_time = -1;
	
	r_ro_single_init(&L.ro, camera.gl_main, r_texture_init_file("res/hare.png", NULL));

}

void hare_update(float dtime) {
	L.x += dtime * L.dx;
	L.y += dtime * L.dy;
	
	if(0){
	static float G = -180;
	
	float ground = tilemap_ground(L.x, L.y);
	printf("y/g: %f/%f\n", L.y, ground);
	
	if(L.y - ground <= 9) {
		L.y = ground + 9;
	}
	if(L.jump_time >= 0.15) {
	    if(L.jump_time < 0.5 && L.speed_y<=0)
	        L.speed_y = 125;
	    L.y += L.speed_y * dtime;
	    L.speed_y += G * dtime;
	    
	    if(L.y - ground <= 3) {
	    	L.jump_time = -1;
		    L.speed_y = 0;
	    	
	    }
	}
	}

	int frame;

	if (0 && L.jump_time >= 0) {
		if(L.jump_time < 0.15)
		    frame = 0;
		else if(L.jump_time < 0.3)
		    frame = 1;
		else if(L.jump_time < 0.5)
		    frame = 2;
		else
		    frame = 3;
		L.jump_time += dtime;
	} else {
		float fps = 6;
		int frames = 4;
		static float time = 0;
		time = fmodf(time + dtime, frames / fps);
		frame = time * fps;
	}

	float w = 1.0 / 4.0;
	float h = 1.0 / 4.0;

	if (L.dx < 0)
		L.looking_left = true;
	if (L.dx > 0)
		L.looking_left = false;

	float v = L.dx == 0 ? 0 : 1;
	if (sca_abs(L.dx) > 40)
		v++;
		
    if(0 && L.jump_time >= 0) 
        v = 3;

    float px = floorf(L.x);
    float py = floorf(L.y);
    
	
	u_pose_set(&L.ro.rect.pose, L.x, L.y, 32, 32, 0);
	
	if(!L.looking_left)
	    u_pose_set(&L.ro.rect.uv, frame * w, v * h, w, h, 0);
	else
	    u_pose_set(&L.ro.rect.uv, (1+frame) * w, v * h, -w, h, 0);
	
}

void hare_render() {
	r_ro_single_render(&L.ro);
}

vec2 hare_position() {
	return (vec2) {{L.x, L.y}};
}

void hare_set_speed(float dx, float dy) {
	// [0 || += 10 : +-80]
	dx = sca_abs(dx) < 10 ? 0 : dx;
	L.dx = sca_sign(dx) * sca_min(sca_abs(dx), 80);
	L.dy = dy;
}

void hare_jump() {
	L.jump_time = 0;
}
