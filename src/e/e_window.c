#include <stdbool.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "e/window.h"
#include "e/definitions.h"

// rhc implementation source file, only once in a project
#include "rhc/rhc_impl.h"


#define MAX_DELTA_TIME 5.0 // seconds


struct eWindowGlobals_s e_window;

typedef struct {
    e_window_pause_callback_fn cb;
    void *ud;    
} RegPause;

static struct {
    bool pause;
    bool running;

    e_window_main_loop_fn main_loop_fn;
    Uint32 last_time;
    
    RegPause reg_pause_e[E_WINDOW_MAX_PAUSE_EVENTS];
    int reg_pause_e_size;
} L;

static void loop() {
    if(L.pause)
        return;
        
    SDL_GetWindowSize(e_window.window, &e_window.size.x, &e_window.size.y);

    Uint32 time = SDL_GetTicks();
    float dtime = (time - L.last_time) / 1000.0f;
    L.last_time = time;

    if(dtime < MAX_DELTA_TIME)
        L.main_loop_fn(dtime);
}


void e_window_init(const char *name) {
#ifdef NDEBUG
    rhc_log_set_min_level(RHC_LOG_WARN);
#else
    rhc_log_set_min_level(RHC_LOG_TRACE);
#endif

    if (SDL_Init(E_SDL_INIT_FLAGS) != 0) {
        log_error("e_window_init: SDL_Init failed: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }


    // initialize IMG
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        log_error("e_window_init: IMG_Init failed: %s", IMG_GetError());
        exit(EXIT_FAILURE);
    }

#ifdef OPTION_TTF
    // initialize TTF
    if (TTF_Init() == -1) {
        log_error("e_window_init: TTF_Init failed: %s", TTF_GetError());
        exit(EXIT_FAILURE);
    }
#endif

    // setup OpenGL usage
    log_info("e_window_init: OpenGL minimal version: %d.%d", E_GL_MAJOR_VERSION, E_GL_MINOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, E_GL_MAJOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, E_GL_MINOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, E_GL_PROFILE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // create window
    e_window.window = SDL_CreateWindow(name,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            640, 480,
            SDL_WINDOW_OPENGL 
            | SDL_WINDOW_RESIZABLE
            );
    SDL_SetWindowMinimumSize(e_window.window, 480, 320);
    if (!e_window.window) {
        log_error("e_window_init: SDL_CreateWindow failed: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Not necessary, but recommended to create a gl context:
    e_window.gl_context = SDL_GL_CreateContext(e_window.window);
    if (!e_window.gl_context) {
        log_error("e_window_init: SDL_GL_CreateContext failed: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_GL_SetSwapInterval(1);  // (0=off, 1=V-Sync, -1=addaptive V-Sync)

#ifdef OPTION_GLEW
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        /* Problem: glewInit failed, something is seriously wrong. */
        log_error( "e_window_init faled: %s", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    log_info("e_window_init: Using GLEW version: %s", glewGetString(GLEW_VERSION));
#endif

    SDL_GetWindowSize(e_window.window, &e_window.size.x, &e_window.size.y);
}

void e_window_kill() {
    log_info("e_window_kill: killing...");
    L.running = false;
    
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#endif
}

void e_window_main_loop(e_window_main_loop_fn main_loop) {
    L.main_loop_fn = main_loop;
    L.pause = false;
    L.running = true;
    L.last_time = SDL_GetTicks();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, true);
#else
    while (L.running)
        loop();
#endif


    SDL_DestroyWindow(e_window.window);
#ifdef OPTION_TTF
    TTF_Quit();
#endif 
    IMG_Quit();
    SDL_Quit();
    log_info("e_window_kill: killed");
}

void e_window_pause() {
    log_info("e_window_pause");
    L.pause = true;
#ifdef __EMSCRIPTEN__
    emscripten_pause_main_loop();
#endif
    for(int i=0; i<L.reg_pause_e_size; i++) {
        L.reg_pause_e[i].cb(false, L.reg_pause_e[i].ud);
    }
}

void e_window_resume() {
    log_info("e_window_resume");
    
    for(int i=0; i<L.reg_pause_e_size; i++) {
        L.reg_pause_e[i].cb(true, L.reg_pause_e[i].ud);
    }
    
    // delta_time should not be near infinity...
    L.last_time = SDL_GetTicks();
    L.pause = false;
#ifdef __EMSCRIPTEN__
    emscripten_resume_main_loop();
#endif
}


void e_window_register_pause_callback(e_window_pause_callback_fn event, void *user_data) {
    assume(L.reg_pause_e_size < E_WINDOW_MAX_PAUSE_EVENTS, "too many registered pause events");
    L.reg_pause_e[L.reg_pause_e_size++] = (RegPause){event, user_data};
}

void e_window_unregister_pause_callback(e_window_pause_callback_fn event_to_unregister) {
    bool found = false;
    for (int i = 0; i < L.reg_pause_e_size; i++) {
        if (L.reg_pause_e[i].cb == event_to_unregister) {
            found = true;
            // move to close hole
            for (int j = i; j < L.reg_pause_e_size - 1; j++) {
                L.reg_pause_e[j] = L.reg_pause_e[j + 1];
            }
            L.reg_pause_e_size--;
            i--; // check moved
        }
    }
    if (!found) {
        log_warn("e_window_unregister_pause_callback failed: event not registered");
    }
}

