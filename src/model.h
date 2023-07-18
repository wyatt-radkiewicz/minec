#ifndef _MODEL_H
#define _MODEL_H
#include <stddef.h>
#include <stdint.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

struct Model;
struct Shader;

// Returns the size of the vertex/instance
typedef size_t(*AttribCreator)(struct Model *model, const struct Shader *shader);
typedef uint16_t VertexIdx;

struct Vertex {
    vec3 pos;
    vec2 uv;
};

struct ModelMatrixInstance {
    mat4 model;
};

struct Model {
    GLuint vao, vbo, ebo, ibo;
    size_t vert_size, inst_size, num_indicies, num_instances;
};

struct Model model_create(AttribCreator vertex_attribs, AttribCreator instance_attribs, const struct Shader *shader);
void model_destroy(struct Model *model);
void model_buffer_vertexes(struct Model *model, const void *elems, size_t count, GLenum usage);
void model_buffer_instances(struct Model *model, const void *insts, size_t count, GLenum usage);
void model_buffer_elements(struct Model *model, const VertexIdx *indexes, size_t count, GLenum usage);
void model_subbuffer_vertexes(struct Model *model, const void *elems, size_t start, size_t count);
void model_subbuffer_instances(struct Model *model, const void *insts, size_t start, size_t count);
void model_subbuffer_elements(struct Model *model, const VertexIdx *indexes, size_t start, size_t count);
void model_bind(struct Model *model);
void model_draw(struct Model *model);

size_t vertex_attrib_creator(struct Model *model, const struct Shader *shader);
size_t model_matrix_attrib_creator(struct Model *model, const struct Shader *shader);

#endif
