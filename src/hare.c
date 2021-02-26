#include "mathc/float.h"
#include "r/ro_single.h"
#include "r/texture.h"
#include "u/pose.h"
#include "camera.h"
#include "tilemap.h"
#include "hare.h"

static struct {
	rRoSingle ro;
	
	rRoSingle coll[8];
	
	vec2 pos;
	vec2 last_pos;
	
	float dx, dy;
	float looking_left;

	float jump_time;
	float speed_y;
} L;


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
		printf("%f - %f %f\n", L.pos.x, a, b);
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
	
	if(L.pos.y < a+14 || L.pos.y < b+14) {
		// grounded
		L.pos.y = sca_max(a, b) + 14;
	}
	
}


void hare_init() {
	L.jump_time = -1;
	
	r_ro_single_init(&L.ro, camera.gl_main, r_texture_init_file("res/hare.png", NULL));

    for(int i=0; i<8; i++)
        r_ro_single_init(&L.coll[i], camera.gl_main, 0);
}

void hare_update(float dtime) {
	L.pos.x += dtime * L.dx;
	L.pos.y += dtime * L.dy;

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

	if (L.dx < 0)
		L.looking_left = true;
	if (L.dx > 0)
		L.looking_left = false;

	float v = L.dx == 0 ? 0 : 1;
	if (sca_abs(L.dx) > 40)
		v++;
		
		
    vec2 p = vec2_mix(L.pos, L.last_pos, 0.5);
	
	u_pose_set(&L.ro.rect.pose, p.x, p.y, 32, 32, 0);	
	if(!L.looking_left)
	    u_pose_set(&L.ro.rect.uv, frame * w, v * h, w, h, 0);
	else
	    u_pose_set(&L.ro.rect.uv, (1+frame) * w, v * h, -w, h, 0);
	
	
	L.last_pos = L.pos;
}

void hare_render() {
	r_ro_single_render(&L.ro);
	for(int i=0; i<8;i++)
	    r_ro_single_render(&L.coll[i]);
}

vec2 hare_position() {
	return (vec2) {{L.pos.x, L.pos.y}};
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
