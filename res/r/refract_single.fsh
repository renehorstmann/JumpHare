in vec2 v_tex_coord;
in vec4 v_color;

out vec4 out_frag_color;

uniform sampler2D tex_main;
uniform sampler2D tex_refract;
uniform sampler2D tex_framebuffer;

void main() {
    out_frag_color = texture(tex_main, v_tex_coord) * v_color;
}
