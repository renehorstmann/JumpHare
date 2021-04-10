#include "r/texture.h"
#include "r/render.h"

struct rRenderGolabals_s r_render;

void r_render_init(SDL_Window *window) {
    r_render.window = window;
    r_render.clear_color = (vec4) {{0, 0, 0, 1}};

    SDL_Log("OpenGL version: %s", glGetString(GL_VERSION));

    int max_vertex_attributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attributes);
    if (max_vertex_attributes < 16) {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "OpenGL failed: only has %d/16 vertex attributes", max_vertex_attributes);
        //exit(EXIT_FAILURE);
    }
    
    // startup "empty" texture
    r_render.framebuffer_tex = r_texture_new_white_pixel();
}

void r_render_begin_frame(int cols, int rows) {
    glViewport(0, 0, cols, rows);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(r_render.clear_color.r, r_render.clear_color.g, r_render.clear_color.b, r_render.clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void r_render_end_frame() {
    SDL_GL_SwapWindow(r_render.window);
}

void r_render_blit_framebuffer(int cols, int rows) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    GLuint tex = r_texture_new_empty(cols, rows);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0); 
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, cols, rows, 0, 0, cols, rows, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    
    // todo: swap n delete
    GLuint tmp = r_render.framebuffer_tex;
    r_render.framebuffer_tex = tex;
    glDeleteTextures(1, &tmp);
}

void r_render_error_check() {
    static GLenum errs[32];
    int errs_size = 0;
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        for (int i = 0; i < errs_size; i++) {
            if (err == errs[i])
                continue;
        }
        SDL_Log("OpenGl error in a frame: 0x%04x", err);
        if (errs_size < 32)
            errs[errs_size++] = err;
    }
}
