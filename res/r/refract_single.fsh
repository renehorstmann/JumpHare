in vec2 v_tex_coord;
in vec4 v_color;

out vec4 out_frag_color;

uniform sampler2D tex_main;
uniform sampler2D tex_refract;
uniform sampler2D tex_framebuffer;

void main() {
    
    // todo:
    // check vec4 gl_FragCoord (position can be reset)
    // use it with tex refracf to get the framebuffer position
    // use alpha of tex_refract
    
    out_frag_color = texture(tex_main, v_tex_coord)
            * texture(tex_refract, v_tex_coord)
            * texture(tex_framebuffer, v_tex_coord)
            * v_color;
}
