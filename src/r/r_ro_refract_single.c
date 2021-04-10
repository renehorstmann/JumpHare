#include "mathc/float.h"
#include "r/shader.h"
#include "r/ro_refract_single.h"


void r_ro_refract_single_init(rRoRefractSingle *self, const float *vp, 
        GLuint tex_main_sink, GLuint tex_refraction_sink, const GLuint *tex_framebuffer_ptr){
    self->rect.pose = mat4_eye();
    self->rect.uv = mat4_eye();
    self->rect.color = vec4_set(1);

    self->vp = vp;

    self->program = r_shader_compile_glsl_from_files((char *[]) {
            "res/r/refract_single.vsh",
            "res/r/refract_single.fsh",
            NULL
    });

    self->tex_main = tex_main_sink;
    self->tex_refraction = tex_refraction_sink;
    self->tex_framebuffer_ptr = tex_framebuffer_ptr;
    self->owns_tex_main = true;
    self->owns_tex_refraction = true;

    // vao scope
    {
        glGenVertexArrays(1, &self->vao);
        glBindVertexArray(self->vao);

        // textures
        glUniform1i(glGetUniformLocation(self->program, "tex_main"), 0);
        
        glUniform1i(glGetUniformLocation(self->program, "tex_refraction"), 1);
        
        glUniform1i(glGetUniformLocation(self->program, "tex_framebuffer"), 2);

        glBindVertexArray(0);
    }
}

void r_ro_refract_single_kill(rRoRefractSingle *self) {
    glDeleteProgram(self->program);
    glDeleteVertexArrays(1, &self->vao);
    if (self->owns_tex_main)
        glDeleteTextures(1, &self->tex_main);
    if (self->owns_tex_refraction)
        glDeleteTextures(1, &self->tex_refraction);
    *self = (rRoRefractSingle) {0};
}


void r_ro_refract_single_render(rRoRefractSingle *self) {
    glUseProgram(self->program);

    glUniformMatrix4fv(glGetUniformLocation(self->program, "pose"), 1, GL_FALSE, &self->rect.pose.m00);

    glUniformMatrix4fv(glGetUniformLocation(self->program, "vp"), 1, GL_FALSE, self->vp);

    glUniformMatrix4fv(glGetUniformLocation(self->program, "uv"), 1, GL_FALSE, &self->rect.uv.m00);

    glUniform4fv(glGetUniformLocation(self->program, "color"), 1, &self->rect.color.v0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, self->tex_main);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, self->tex_refraction);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, *self->tex_framebuffer_ptr);

    {
        glBindVertexArray(self->vao);
        // r_shader_validate(self->program); // debug test
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    glUseProgram(0);
}

void r_ro_refract_single_set_texture_main(rRoRefractSingle *self, GLuint tex_main_sink) {
    if (self->owns_tex_main)
        glDeleteTextures(1, &self->tex_main);
    self->tex_main = tex_main_sink;
}

void r_ro_refract_single_set_texture_refraction(rRoRefractSingle *self, GLuint tex_refraction_sink){
    if (self->owns_tex_refraction)
        glDeleteTextures(1, &self->tex_refraction);
    self->tex_refraction = tex_refraction_sink;
}
