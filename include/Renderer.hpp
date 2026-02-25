#pragma once

#include "Vec2.hpp"
#include "Color.hpp"

class Texture;
struct Transform;

namespace renderer
{
    void clear(const Color &color = {0, 0, 0, 255});
    void present();

    void draw(const Texture &texture, const Transform &transform, const Vec2 &anchor = {}, const Vec2 &pivot = {});

    void _init(SDL_Window *window, const int width, const int height);
    void _quit();
    SDL_Renderer *_get();
}
