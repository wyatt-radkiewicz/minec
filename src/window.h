#ifndef _WINDOW_H
#define _WINDOW_H
#include <stdbool.h>
#include <SDL.h>

struct Window {
    SDL_Window *window;
    SDL_GLContext context;
    bool wants_to_close;
    int error_code;
    bool sdl_initialized;

    bool lock_mouse;
    float mouse_dx, mouse_dy;

    uint32_t w, h;
};

struct Window window_create(void);
void window_destroy(struct Window *window);
void window_handle_events(struct Window *window);

#endif
