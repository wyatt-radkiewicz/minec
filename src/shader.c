#include <stdlib.h>
#include <string.h>

#include "shader.h"

static GLuint compile_shader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    if (!shader) {
        printf("shader error: Can not create shader!\n");
        return 0;
    }
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

    if (!compile_status) {
        GLint max_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

        char *msg = malloc(max_length);
        assert(msg && "Not enough memory!");
        glGetShaderInfoLog(shader, max_length, &max_length, msg);
        switch (type) {
        case GL_VERTEX_SHADER: printf("vertex"); break;
        case GL_FRAGMENT_SHADER: printf("fragment"); break;
        case GL_GEOMETRY_SHADER: printf("geometry"); break;
        default: break;
        }
        printf(" shader error: %s\n", msg);

        glDeleteShader(shader);
        free(msg);
        return 0;
    } else {
        return shader;
    }
}

struct ShaderResult shader_create(const char *vertex_source,
                                    const char *fragment_source,
                                    UniformLoader uniforms) {
    GLuint vertex = compile_shader(GL_VERTEX_SHADER, vertex_source);
    GLuint fragment = compile_shader(GL_FRAGMENT_SHADER, fragment_source);

    struct ShaderResult result = (struct ShaderResult) {
        .valid = false,
        .program = {
            .program = 0,
        },
    };
    if (!vertex || !fragment) {
        goto cleanup;
    }

    result.program.program = glCreateProgram();
    if (!result.program.program) {
        printf("shader error: Can not create shader program!\n");
        goto cleanup;
    }

    glAttachShader(result.program.program, vertex);
    glAttachShader(result.program.program, fragment);
    glLinkProgram(result.program.program);
    
    GLint link_status;
    glGetProgramiv(result.program.program, GL_LINK_STATUS, &link_status);

    if (!link_status) {
        GLint max_length = 0;
        glGetProgramiv(result.program.program, GL_INFO_LOG_LENGTH, &max_length);

        char *msg = malloc(max_length);
        assert(msg && "Not enough memory!");
        glGetShaderInfoLog(result.program.program, max_length, &max_length, msg);
        printf("shader error: %s\n", msg);

        glDeleteProgram(result.program.program);
        result.program.program = 0;
        free(msg);
        goto cleanup;
    } else {
        result.valid = true;
    }

    if (uniforms) {
        uniforms(&result.program);
    }

cleanup:
    if (vertex) {
        glDeleteShader(vertex);
    }
    if (fragment) {
        glDeleteShader(fragment);
    }
    return result;
}
void shader_destroy(struct Shader *program) {
    if (program->program) {
        glDeleteProgram(program->program);
        program->program = 0;
    }
}
void shader_use(struct Shader *program) {
    if (program) {
        glUseProgram(program->program);
    } else {
        glUseProgram(0);
    }
}
void shader_set_binding(struct Shader *program,
                            const void *_buffer,
                            const char *uniform) {
    struct UniformBuffer *buffer = (struct UniformBuffer *)_buffer - 1;
    GLuint idx = glGetUniformBlockIndex(program->program, uniform);
    if (idx) {
        glUniformBlockBinding(program->program, idx, buffer->binding);
    }
}

void textured_shader_uniforms(struct Shader *shader) {
    shader->uniforms.textured.texture =
        glGetUniformLocation(shader->program, "diffuse");
}

void *uniformbuffer_create(GLuint binding, size_t inst_size, size_t inst_count, GLenum usage) {
    struct UniformBuffer *buffer = malloc(sizeof(struct UniformBuffer) + inst_size * inst_count);

    buffer->binding = binding;
    buffer->usage = usage;
    buffer->inst_size = inst_size;
    buffer->inst_count = inst_count;
    buffer->inst_capacity = inst_count;
    memset(buffer->buffer, 0, buffer->inst_size * buffer->inst_capacity);

    glGenBuffers(1, &buffer->ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer->ubo);
    glBufferData(
        GL_UNIFORM_BUFFER,
        buffer->inst_size * buffer->inst_capacity,
        buffer->buffer,
        buffer->usage
    );

    return buffer->buffer;
}
void uniformbuffer_destroy(void *_buffer) {
    struct UniformBuffer *buffer = (struct UniformBuffer *)_buffer - 1;
    glDeleteBuffers(1, &buffer->ubo);
    free(buffer);
}
void *uniformbuffer_resize(void *_buffer, size_t inst_count) {
    struct UniformBuffer *buffer = (struct UniformBuffer *)_buffer - 1;
    buffer->inst_count = inst_count;
    if (inst_count > buffer->inst_capacity) {
        while (buffer->inst_capacity < inst_count) {
            buffer->inst_capacity *= 2;
        }
    } else if (inst_count < buffer->inst_capacity / 10) {
        buffer->inst_capacity = inst_count * 2;
    } else {
        return buffer->buffer;
    }
    buffer = realloc(buffer, sizeof(*buffer) + buffer->inst_capacity * buffer->inst_size);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer->ubo);
    glBufferData(GL_UNIFORM_BUFFER, buffer->inst_size * buffer->inst_capacity, buffer->buffer, buffer->usage);
    return buffer->buffer;
}
void uniformbuffer_bind(void *_buffer, size_t instance) {
    struct UniformBuffer *buffer = (struct UniformBuffer *)_buffer - 1;
    glBindBufferRange(
        GL_UNIFORM_BUFFER,
        buffer->binding,
        buffer->ubo,
        instance * buffer->inst_size,
        buffer->inst_size
    );
}
void uniformbuffer_update_instances(void *_buffer, size_t start, size_t num) {
    struct UniformBuffer *buffer = (struct UniformBuffer *)_buffer - 1;
    glBindBuffer(GL_UNIFORM_BUFFER, buffer->ubo);
    glBufferSubData(
        GL_UNIFORM_BUFFER,
        start * buffer->inst_size,
        buffer->inst_size * num,
        buffer->buffer + buffer->inst_size * start
    );
}
