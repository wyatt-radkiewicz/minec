#ifndef _SHADER_H
#define _SHADER_H
#include <stdbool.h>
#include <stdint.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

struct Shader;
struct UniformBuffer;
typedef void(*UniformLoader)(struct Shader *shader);
struct TexturedShader {
    GLint texture;
};

struct Shader {
    GLuint program;
    union {
        struct TexturedShader textured;
    } uniforms;
};
struct ShaderResult {
    struct Shader program;
    bool valid;
};

struct ShaderResult shader_create(const char *vertex_source,
                                    const char *fragment_source,
                                    UniformLoader uniforms);
void shader_destroy(struct Shader *program);
void shader_use(struct Shader *program);
void shader_set_binding(struct Shader *program,
                            const void *buffer,
                            const char *uniform);

void textured_shader_uniforms(struct Shader *shader);

static inline void shader_set_matrix(GLint uniform_loc, mat4 matrix) {
    glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, (void *)matrix);
}
static inline void shader_set_vec4(GLint uniform_loc, vec4 vec) {
    glUniform4f(uniform_loc, vec[0], vec[1], vec[2], vec[3]);
}
static inline void shader_set_vec3(GLint uniform_loc, vec3 vec) {
    glUniform3f(uniform_loc, vec[0], vec[1], vec[2]);
}
static inline void shader_set_vec2(GLint uniform_loc, vec2 vec) {
    glUniform2f(uniform_loc, vec[0], vec[1]);
}
static inline void shader_set_int(GLint uniform_loc, int val) {
    glUniform1i(uniform_loc, val);
}
static inline void shader_set_float(GLint uniform_loc, float val) {
    glUniform1f(uniform_loc, val);
}

struct UniformMatrices {
    mat4 proj;
    mat4 view;
};
struct UniformBuffer {
    GLenum usage;
    GLuint binding, ubo;
    size_t inst_size, inst_count, inst_capacity;
    uint8_t buffer[];
};

void *uniformbuffer_create(GLuint binding, size_t inst_size, size_t inst_count, GLenum usage);
void uniformbuffer_destroy(void *buffer);
static inline size_t uniformbuffer_len(void *_buffer) {
    struct UniformBuffer *buffer = (struct UniformBuffer *)_buffer - 1;
    return buffer->inst_count;
}
void *uniformbuffer_resize(void *buffer, size_t inst_count);
void uniformbuffer_bind(void *buffer, size_t inst_idx);
void uniformbuffer_update_instances(void *buffer, size_t start, size_t num);

#endif
