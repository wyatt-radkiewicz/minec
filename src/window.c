#include <stdio.h>
#include <glad/glad.h>

#include "SDL_video.h"
#include "window.h"

struct Window window_create(void) {
    struct Window window = (struct Window) {
        .window = NULL,
        .error_code = -1,
        .sdl_initialized = false,
        .w = 800,
        .h = 600,
    };

    if ((window.error_code = SDL_Init(SDL_INIT_VIDEO |
                                    SDL_INIT_AUDIO |
                                    SDL_INIT_GAMECONTROLLER)) != 0) {
        printf("SDL2 error: %s\n", SDL_GetError());
        return window;
    } else {
        window.sdl_initialized = true;
    }

    // Create main window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    if ((window.window = SDL_CreateWindow(
        "MineC",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        window.w, window.h,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    )) == NULL) {
        printf("SDL2 error: %s\n", SDL_GetError());
        return window;
    }

    // Set OpenGL context
    if ((window.context = SDL_GL_CreateContext(window.window)) == NULL) {
        printf("SDL2 error: %s\n", SDL_GetError());
        return window;
    }
    SDL_GL_MakeCurrent(window.window, window.context);
    SDL_GL_SetSwapInterval(-1);

    // Get the OpenGL context
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("glad error: Can't load the GL load procedure!\n");
        return window;
    }

    window.error_code = 0;
    return window;
}

void window_destroy(struct Window *window) {
    if (window->window) {
        SDL_DestroyWindow(window->window);
        window->window = NULL;
    }

    if (window->context) {
        SDL_GL_DeleteContext(window->context);
    }

    if (window->sdl_initialized) {
        SDL_Quit();
    }
}
void window_handle_events(struct Window *window) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            window->wants_to_close = true;
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                glViewport(0, 0, event.window.data1, event.window.data2);
                window->w = event.window.data1;
                window->h = event.window.data2;
            }
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                window->lock_mouse = false;
            }
            break;
        default: break;
        }
    }

    if (window->lock_mouse) {
        int x, y;
        SDL_GetRelativeMouseState(&x, &y);
        window->mouse_dx = (float)x / (float)window->w;
        window->mouse_dy = (float)y / (float)window->h;
        SDL_WarpMouseInWindow(window->window, window->w / 2, window->h / 2);
    } else {
        window->mouse_dx = 0.0f;
        window->mouse_dy = 0.0f;
    }
}
