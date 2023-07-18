#include <cglm/cglm.h>
#include "camera.h"

struct Camera camera_create(float fovy, float nearz, float farz) {
    struct Camera cam = (struct Camera) {
        .pos = { 0.0f, 0.0f, 0.0f },
        .rot = { 1.0f, 0.0f, 0.0f, 0.0f },
        .fovy = fovy,
        .nearz = nearz,
        .farz = farz,
        .xrot = 0.0f,
        .yrot = 0.0f,
        .use_xy = true,
    };
    return cam;
}
