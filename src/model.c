#include "model.h"
#include "shader.h"

struct Model model_create(AttribCreator vertex_attribs,
                            AttribCreator instance_attribs,
                            const struct Shader *shader) {
    struct Model model = (struct Model){
        .vao = 0,
        .vbo = 0,
        .ebo = 0,
        .ibo = 0,
        .vert_size = sizeof(float)*3,
        .inst_size = 0,
        .num_indicies = 0,
        .num_instances = 0,
    };

    glGenVertexArrays(1, &model.vao);
    if (vertex_attribs) {
        glGenBuffers(1, &model.vbo);
        glGenBuffers(1, &model.ebo);
    }
    if (instance_attribs) {
        glGenBuffers(1, &model.ibo);
    }
    glBindVertexArray(model.vao);
    glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ebo);

    if (vertex_attribs) {
        model.vert_size = vertex_attribs(&model, shader);
    }
    if (instance_attribs) {
        glBindBuffer(GL_ARRAY_BUFFER, model.ibo);
        model.inst_size = instance_attribs(&model, shader);
    }
    glBindVertexArray(0);
    
    return model;
}
void model_destroy(struct Model *model) {
    assert(model);
    if (model->ebo) {
        glDeleteBuffers(1, &model->ebo);
    }
    if (model->vbo) {
        glDeleteBuffers(1, &model->vbo);
    }
    if (model->ibo) {
        glDeleteBuffers(1, &model->ibo);
    }
    if (model->vao) {
        glDeleteVertexArrays(1, &model->vao);
    }
}
void model_buffer_vertexes(struct Model *model, const void *elems, size_t count, GLenum usage) {
    assert(model->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
    glBufferData(GL_ARRAY_BUFFER, model->vert_size * count, elems, usage);
}
void model_buffer_instances(struct Model *model, const void *insts, size_t count, GLenum usage) {
    assert(model->ibo);
    model->num_instances = count;
    glBindBuffer(GL_ARRAY_BUFFER, model->ibo);
    glBufferData(GL_ARRAY_BUFFER, model->inst_size * count, insts, usage);
}
void model_buffer_elements(struct Model *model, const VertexIdx *indexes, size_t count, GLenum usage) {
    assert(model->ebo);
    model->num_indicies = count;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indexes) * count, indexes, usage);
}
void model_subbuffer_vertexes(struct Model *model, const void *elems, size_t start, size_t count) {
    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, model->vert_size * start, model->vert_size * count, elems);
}
void model_subbuffer_instances(struct Model *model, const void *insts, size_t start, size_t count) {
    glBindBuffer(GL_ARRAY_BUFFER, model->ibo);
    glBufferSubData(GL_ARRAY_BUFFER, model->inst_size * start, model->inst_size * count, insts);
    
}
void model_subbuffer_elements(struct Model *model, const VertexIdx *indexes, size_t start, size_t count) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indexes) * start, sizeof(*indexes) * count, indexes);
}
void model_bind(struct Model *model) {
    if (model) {
        glBindVertexArray(model->vao);
    } else {
        glBindVertexArray(0);
    }
}
void model_draw(struct Model *model) {
    if (!model->num_instances) {
        glDrawElements(GL_TRIANGLES, model->num_indicies, GL_UNSIGNED_SHORT, 0);
    } else {
        glDrawElementsInstanced(GL_TRIANGLES, model->num_indicies, GL_UNSIGNED_SHORT, 0, model->num_instances);
    }
}

size_t vertex_attrib_creator(struct Model *model, const struct Shader *shader) {
    GLint pos_loc = glGetAttribLocation(shader->program, "in_pos");
    GLint uv_loc = glGetAttribLocation(shader->program, "in_uv");
    if (pos_loc != -1) {
        glEnableVertexAttribArray(pos_loc);
        glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)0);
    }
    if (uv_loc != -1) {
        glEnableVertexAttribArray(uv_loc);
        glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, uv));
    }
    return sizeof(struct Vertex);
}
size_t model_matrix_attrib_creator(struct Model *model, const struct Shader *shader) {
    GLint model_loc = glGetAttribLocation(shader->program, "in_model");
    if (model_loc != -1) {
        for (size_t i = 0; i < 4; i++) {
            glEnableVertexAttribArray(model_loc + i);
            glVertexAttribPointer(model_loc + i, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void *)(i*sizeof(vec4)));
            glVertexAttribDivisor(model_loc + i, 1);
        }
    }
    return sizeof(mat4);
}
