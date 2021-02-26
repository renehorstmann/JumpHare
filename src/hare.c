#include "mathc/float.h"
#include "r/ro_single.h"
#include "r/texture.h"
#include "u/pose.h"
#include "camera.h"
#include "tilemap.h"
#include "hare.h"

static struct {
	rRoSingle ro;
	
	rRoSingle coll[4];
	
	float x, y;
	
	float dx, dy;
	float looking_left;

	float jump_time;
	float speed_y;
} L;

void hare_init() {
	L.jump_time = -1;
	
	r_ro_single_init(&L.ro, camera.gl_main, r_texture_init_file("res/hare.png", NULL));

    for(int i=0; i<4; i++)
        r_ro_single_init(&L.coll[i], camera.gl_main, 0);
}

void hare_update(float dtime) {
	L.x += dtime * L.dx;
	L.y += dtime * L.dy;

	float ground = tilemap_ground(L.x, L.y-4);
	L.coll[0].rect.pose = u_pose_new(L.x, ground, 4, 4);
	
	if(L.y < ground + 8) {
	    L.y = ground + 8;
	    // grounded
	}
	
	float wleft = tilemap_wall_left(L.x-4, L.y);
	L.coll[1].rect.pose = u_pose_new(wleft, L.y, 4, 4);
	
	
	if(L.x < wleft + 8)
	    L.x = wleft + 8;
	
	float wright = tilemap_wall_right(L.x+4, L.y);
	L.coll[2].rect.pose = u_pose_new(wright, L.y, 4, 4);
	
	if(L.x > wright - 8)
	    L.x = wright - 8;
	
	    
	
	float ceiling = tilemap_ceiling(L.x, L.y+4);
	L.coll[3].rect.pose = u_pose_new(L.x, ceiling, 4, 4);
	
	if(L.y > ceiling - 8)
	    L.y = ceiling - 8;
	
	
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

	if (L.dx < 0)
		L.looking_left = true;
	if (L.dx > 0)
		L.looking_left = false;

	float v = L.dx == 0 ? 0 : 1;
	if (sca_abs(L.dx) > 40)
		v++;
		
	
	u_pose_set(&L.ro.rect.pose, L.x, L.y, 32, 32, 0);
	
	if(!L.looking_left)
	    u_pose_set(&L.ro.rect.uv, frame * w, v * h, w, h, 0);
	else
	    u_pose_set(&L.ro.rect.uv, (1+frame) * w, v * h, -w, h, 0);
	
}

void hare_render() {
	r_ro_single_render(&L.ro);
	for(int i=0; i<4;i++)
	    r_ro_single_render(&L.coll[i]);
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
