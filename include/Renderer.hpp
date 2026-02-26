#pragma once

#include <cstddef>
#include "Vec2.hpp"
#include "Color.hpp"

class Texture;
struct Transform;

namespace renderer
{
    void clear(const Color &color = {0, 0, 0, 255});
    void present();

    void draw(const Texture &texture, const Transform &transform, const Vec2 &anchor = {}, const Vec2 &pivot = {});

    // Batch render from SoA arrays (no Transform construction needed)
    void draw_batch_soa(
        const Texture &texture,
        const double *pos_x, const double *pos_y,
        const double *rot,
        const double *scale_x, const double *scale_y,
        size_t count,
        const Vec2 &anchor = {}, const Vec2 &pivot = {});

    void _init(SDL_Window *window, const int width, const int height);
    void _quit();
    SDL_Renderer *_get();
}
