#ifndef _CAMERA_H
#define _CAMERA_H
#include <string.h>
#include <math.h>
#include <cglm/cglm.h>

#include "window.h"

struct Camera {
    vec3 pos;
    vec4 rot;
    float xrot, yrot;
    float fovy, aspect;
    float nearz, farz;
    bool use_xy;
};

static inline void quat_from_euler_xyz(vec3 xyz, vec4 dest) {
    float cr = cosf(xyz[0] * 0.5);
    float sr = sinf(xyz[0] * 0.5);
    float cp = cosf(xyz[1] * 0.5);
    float sp = sinf(xyz[1] * 0.5);
    float cy = cosf(xyz[2] * 0.5);
    float sy = sinf(xyz[2] * 0.5);

    dest[3] = cr * cp * cy + sr * sp * sy;
    dest[0] = sr * cp * cy - cr * sp * sy;
    dest[1] = cr * sp * cy + sr * cp * sy;
    dest[2] = cr * cp * sy - sr * sp * cy;
}

static inline void quat_to_euler_xyz(vec4 q, vec3 e) {
    float sinr_cosp = 2.0f * (q[3] * q[0] + q[1] * q[2]);
    float cosr_cosp = 1.0f - 2.0f * (q[0] * q[0] + q[1] * q[1]);
    e[0] = atan2f(sinr_cosp, cosr_cosp);

    float sinp = sqrtf(1.0f + 2.0f * (q[3] * q[1] - q[0] * q[2]));
    float cosp = sqrtf(1.0f - 2.0f * (q[3] * q[1] - q[0] * q[2]));
    e[1] = 2.0f * atan2f(sinp, cosp) - M_PI / 2.0f;

    float siny_cosp = 2.0f * (q[3] * q[2] + q[0] * q[1]);
    float cosy_cosp = 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]);
    e[2] = atan2f(siny_cosp, cosy_cosp);
}

struct Camera camera_create(float fovy, float nearz, float farz);
static inline void camera_update(struct Camera *cam, struct Window *window) {
    cam->aspect = (float)window->w / (float)window->h;
}
static inline void camera_get_proj(struct Camera *cam, mat4 proj) {
    glm_perspective(cam->fovy, cam->aspect, cam->nearz, cam->farz, proj);
}
static inline void camera_get_view(struct Camera *cam, mat4 view) {
    if (cam->use_xy) {
        glm_mat4_identity(view);
        glm_rotate_x(view, -cam->xrot, view);
        glm_rotate_y(view, -cam->yrot, view);
        glm_translate(view, (vec3){ -cam->pos[0], -cam->pos[1], -cam->pos[2] });
    } else {
        glm_quat_look(cam->pos, cam->rot, view);
    }
}
static inline void camera_get_mat(struct Camera *cam, mat4 mat) {
    mat4 proj, view;
    camera_get_proj(cam, proj);
    camera_get_view(cam, view);
    glm_mat4_mul(proj, view, proj);
    memcpy(mat, proj, sizeof(mat4));
}

#endif
