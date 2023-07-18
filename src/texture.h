#ifndef _TEXTURE_H
#define _TEXTURE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <glad/glad.h>

struct Texture {
    uint32_t w, h;
    bool mipmaps;
    GLuint texture;

    bool is_array;
    uint32_t num_textures;
};

struct Image {
    uint32_t w, h;
    uint8_t *data;
    size_t bytes_per_pixel, pitch;

    bool _stbi_loaded;
};

// TODO: Will only work on Little Endian systems (I think)
// rgba8888 (on little endian its abgr32, and rgba32 on big endian)
union Color {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint8_t comps[4];
    uint32_t pixel;
};

struct Texture texture_create_from_file(const char *path, GLenum sampling, bool mipmaps);
struct Texture texture_create_empty(uint32_t width, uint32_t height, GLenum sampling, bool mipmaps);
void texture_draw_image(struct Texture *texture, const struct Image *img, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void texture_destroy(struct Texture *texture);
void texture_activate(struct Texture *texture, uint32_t slot);

struct Texture texture_array_create_empty(GLenum sampling, bool mipmaps, uint32_t num_textures, uint32_t w, uint32_t h);
void texture_array_load_image(struct Texture *tarray, uint32_t idx, const struct Image *img);
void texture_array_load_subimage(struct Texture *tarray, uint32_t idx, const struct Image *img, uint32_t srcx, uint32_t srcy);

struct Image image_create_from_file(const char *path);
struct Image image_create_empty(uint32_t width, uint32_t height);
void image_destroy(struct Image *image);
void image_draw(struct Image *dest, const struct Image *source, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t srcx, uint32_t srcy);
static inline const union Color *image_get_base_ptr(const struct Image *image, uint32_t x, uint32_t y) {
    return (const union Color *)(image->data + (y * image->pitch + x * 4));
}

#endif
