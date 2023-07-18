#include <stddef.h>
#include <math.h>

#include "window.h"
#include "shader.h"
#include "model.h"
#include "file.h"
#include "mem.h"
#include "texture.h"
#include "system.h"
#include "camera.h"

struct Vertex verticies[] = {
    { .pos = {-0.5f,  0.5f,  0.0f, }, .uv = { 0.0f, 0.0f, }, },
    { .pos = { 0.5f,  0.5f,  0.0f, }, .uv = { 1.0f, 0.0f, }, },
    { .pos = {-0.5f, -0.5f,  0.0f, }, .uv = { 0.0f, 1.0f, }, },
    { .pos = { 0.5f, -0.5f,  0.0f, }, .uv = { 1.0f, 1.0f, }, },
};
VertexIdx indicies[] = {
    2, 1, 0,
    2, 3, 1,
};

int main(int argc, char **argv) {
    struct Window window = window_create();
    struct Arena *arena = arena_create(1024 * 1024);
    if (window.error_code != 0) {
        return window.error_code;
    }

    // Application
    glEnable(GL_DEPTH_TEST);
    char *vertex = file_load_as_string(arena_alloc_interface(), arena, "assets/shaders/test.vs"),
        *fragment = file_load_as_string(arena_alloc_interface(), arena, "assets/shaders/test.fs");
    struct ShaderResult shader_result = shader_create(vertex, fragment, textured_shader_uniforms);
    if (!shader_result.valid) {
        goto cleanup;
    }
    shader_use(&shader_result.program);
    shader_set_int(shader_result.program.uniforms.textured.texture, 0);
    struct Model model = model_create(vertex_attrib_creator, model_matrix_attrib_creator, &shader_result.program);
    model_buffer_vertexes(&model, verticies, ARRAY_SIZE(verticies), GL_STATIC_DRAW);
    model_buffer_elements(&model, indicies, ARRAY_SIZE(indicies), GL_STATIC_DRAW);
    struct Texture terrain = texture_array_create_empty(GL_NEAREST, true, 25, 16, 16);
    struct UniformMatrices *matricies = uniformbuffer_create(0, sizeof(struct UniformMatrices), 1, GL_STREAM_DRAW);
    struct Camera cam = camera_create(glm_rad(70.0f), 0.001f, 10000.0f);

    struct Image terrain_img = image_create_from_file("assets/textures/terrain.png");
    for (int i = 0; i < 25; i++) {
        texture_array_load_subimage(
            &terrain,
            i,
            &terrain_img,
            (i % 16) * 16,
            (i / 16) * 16
        );
    }
    texture_draw_image(&terrain, &terrain_img, 0, 0, 256, 256);
    image_destroy(&terrain_img);

    mat4 models[5*5];
    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            mat4 *m = &models[(y + 2) * 5 + (x + 2)];
            glm_mat4_identity(*m);
            glm_translate(*m, (vec3){ (float)x * 1.3f, (float)y * 1.3f, -4.0f });
        }
    }
    model_buffer_instances(&model, models, 25, GL_STATIC_DRAW);

    texture_activate(&terrain, 0);
    shader_use(&shader_result.program);
    uniformbuffer_bind(matricies, 0);
    shader_set_binding(&shader_result.program, matricies, "Matrices");
    model_bind(&model);
    window.lock_mouse = true;

    while (!window.wants_to_close) {
        window_handle_events(&window);

        const uint8_t *keys = SDL_GetKeyboardState(NULL);
        float movespd = (float)((keys[SDL_SCANCODE_W] != 0) -
            (keys[SDL_SCANCODE_S] != 0)) * 0.05f;
        float movespd2 = (float)((keys[SDL_SCANCODE_A] != 0) -
            (keys[SDL_SCANCODE_D] != 0)) * 0.04f;

        // Move the camera
        cam.xrot -= window.mouse_dy;
        cam.yrot -= window.mouse_dx;
        
        vec3 cam_normal = { 0.0f, 0.0f, -1.0f }, cam_tangent;
        glm_vec3_rotate(cam_normal, cam.xrot, (vec3){ 1.0f, 0.0f, 0.0f });
        glm_vec3_rotate(cam_normal, cam.yrot, (vec3){ 0.0f, 1.0f, 0.0f });
        glm_vec3_cross(cam_normal, (vec3){ 0.0f, 1.0f, 0.0f }, cam_tangent);
        glm_vec3_scale(cam_normal, movespd, cam_normal);
        glm_vec3_add(cam_normal, cam.pos, cam.pos);
        glm_vec3_scale(cam_tangent, -movespd2, cam_tangent);
        glm_vec3_add(cam_tangent, cam.pos, cam.pos);

        camera_update(&cam, &window);
        camera_get_proj(&cam, matricies[0].proj);
        camera_get_view(&cam, matricies[0].view);
        uniformbuffer_update_instances(matricies, 0, 1);

        glClearColor(0.2f, 0.5f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        model_bind(&model);
        model_draw(&model);

        SDL_GL_SwapWindow(window.window);
    }

cleanup_resources:
    uniformbuffer_destroy(matricies);
    shader_destroy(&shader_result.program);
    model_destroy(&model);
    texture_destroy(&terrain);

cleanup:
    arena_destroy(arena);
    window_destroy(&window);
    return window.error_code;
}
