#pragma once

#include <string>

#include <nanobind/nanobind.h>
#include <SDL3/SDL.h>

#include "Rect.hpp"

namespace nb = nanobind;

class Texture
{
public:
    struct Flip
    {
        bool h{false};
        bool v{false};
    } flip;

    explicit Texture(const std::string &file_path);
    ~Texture();

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    Vec2 getSize() const { return {m_width, m_height}; }

    Rect getClipArea() const { return m_clipArea; }
    void setClipArea(const Rect &area) { m_clipArea = area; }

    float getAlpha() const;
    void setAlpha(float alpha) const;

    SDL_Texture *getSDL() const { return m_texture; }

private:
    SDL_Texture *m_texture;
    int m_width{0};
    int m_height{0};
    Rect m_clipArea{};
};
