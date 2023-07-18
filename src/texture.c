#ifdef __APPLE__
# include <TargetConditionals.h>
# if TARGET_OS_SIMULATOR != 1
#  define STBI_NEON
# endif
#endif
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "texture.h"

struct Texture texture_create_from_file(const char *path, GLenum sampling, bool mipmaps) {
    int x, y, n;
    unsigned char *data = stbi_load(path, &x, &y, &n, 4);
    if (!data) {
        printf("texture error: Can not load the image file %s\n", path);
        return (struct Texture) {
            .texture = 0,
            .w = 0,
            .h = 0,
            .mipmaps = mipmaps,
            .is_array = false,
            .num_textures = 1,
        };
    }
    assert(n == 4);
    
    struct Texture texture = (struct Texture) {
        .w = x,
        .h = y,
        .mipmaps = mipmaps,
        .is_array = false,
        .num_textures = 1,
    };
    glGenTextures(1, &texture.texture);
    glBindTexture(GL_TEXTURE_2D, texture.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampling);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    if (mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    return texture;
}
struct Texture texture_create_empty(uint32_t width, uint32_t height, GLenum sampling, bool mipmaps) {
    struct Texture texture = (struct Texture) {
        .w = width,
        .h = height,
        .mipmaps = mipmaps,
        .is_array = false,
        .num_textures = 1,
    };

    glGenTextures(1, &texture.texture);
    glBindTexture(GL_TEXTURE_2D, texture.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampling);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    if (mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    return texture;
}
void texture_draw_image(struct Texture *texture, const struct Image *img, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    assert(texture && img);
    if (!texture->texture) {
        printf("image error: Drawing to 0 texture!\n");
        return;
    }

    glBindTexture(GL_TEXTURE_2D, texture->texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
    if (texture->mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}
void texture_destroy(struct Texture *texture) {
    assert(texture);
    if (texture->texture) {
        glDeleteTextures(1, &texture->texture);
    }
}
void texture_activate(struct Texture *texture, uint32_t slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    if (texture->is_array) {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture->texture);
    } else {
        glBindTexture(GL_TEXTURE_2D, texture->texture);
    }
}

struct Texture texture_array_create_empty(GLenum sampling, bool mipmaps, uint32_t num_textures, uint32_t w, uint32_t h) {
    struct Texture texture = (struct Texture) {
        .w = w,
        .h = h,
        .mipmaps = mipmaps,
        .is_array = true,
        .num_textures = num_textures,
    };

    glGenTextures(1, &texture.texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture.texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, sampling);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, mipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, w, h, num_textures, GL_FALSE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    if (mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    return texture;
    
}
void texture_array_load_image(struct Texture *tarray, uint32_t idx, const struct Image *img) {
    assert(tarray && img && tarray->w == img->w && tarray->h == img->h);
    if (!tarray->texture) {
        printf("image error: Drawing to 0 texture!\n");
        return;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, tarray->texture);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, idx, tarray->w, tarray->h, 4, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
    if (tarray->mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }
}
void texture_array_load_subimage(struct Texture *tarray, uint32_t idx, const struct Image *img, uint32_t srcx, uint32_t srcy) {
    assert(tarray && img);
    if (!tarray->texture) {
        printf("image error: Drawing to 0 texture!\n");
        return;
    }

    struct Image sub = image_create_empty(tarray->w, tarray->h);
    image_draw(&sub, img, 0, 0, tarray->w, tarray->h, srcx, srcy);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tarray->texture);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, idx, tarray->w, tarray->h, 1, GL_RGBA, GL_UNSIGNED_BYTE, sub.data);
    if (tarray->mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    image_destroy(&sub);
}

struct Image image_create_from_file(const char *path) {
    int x, y, n;
    unsigned char *data = stbi_load(path, &x, &y, &n, 4);
    assert(n == 4);
    if (!data) {
        printf("image error: Failed to load the image file %s\n", path);
        return (struct Image) {
            .w = 0,
            .h = 0,
            .data = NULL,
            .bytes_per_pixel = 4,
            .pitch = 0,
            ._stbi_loaded = false,
        };
    }

    return (struct Image) {
        .w = x,
        .h = y,
        .data = data,
        .bytes_per_pixel = 4,
        .pitch = 4 * x,
        ._stbi_loaded = true,
    };
}
struct Image image_create_empty(uint32_t width, uint32_t height) {
    return (struct Image) {
        .w = width,
        .h = height,
        .data = calloc(width * height, 4),
        .bytes_per_pixel = 4,
        .pitch = 4 * width,
        ._stbi_loaded = false,
    };
}
void image_destroy(struct Image *image) {
    assert(image);
    if (image->data) {
        if (image->_stbi_loaded) {
            stbi_image_free(image->data);
        } else {
            free(image->data);
        }
    }
}
void image_draw(struct Image *dest, const struct Image *source, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t srcx, uint32_t srcy) {
    if (x >= dest->w) {
        x = dest->w - 1;
    }
    if (x + w > dest->w) {
        w = dest->w - x;
    }
    if (srcx >= source->w) {
        srcx = source->w - 1;
    }
    if (srcx + w > source->w) {
        uint32_t new = source->w - srcx;
        w = new < w ? new : w;
    }

    if (y >= dest->h) {
        y = dest->h - 1;
    }
    if (y + h > dest->h) {
        h = dest->h - y;
    }
    if (srcy >= source->h) {
        srcy = source->h - 1;
    }
    if (srcy + h > source->h) {
        uint32_t new = source->h - srcy;
        h = new < h ? new : h;
    }

    for (uint32_t i = 0; i < h; i++) {
        const union Color *src = image_get_base_ptr(source, srcx, i + srcy);
        union Color *dst = (union Color *)image_get_base_ptr(dest, x, i + y);
        for (uint32_t j = 0; j < w; j++, dst++, src++) {
            *dst = *src;
        }
    }
}
