#pragma once

#include <SDL3/SDL.h>

#include "Vec2.hpp"

class Rect
{
public:
    double x{0.0};
    double y{0.0};
    double w{0.0};
    double h{0.0};

    Rect() = default;

    template <typename Number>
    Rect(Number x, Number y, Number w, Number h)
        : x(static_cast<double>(x)), y(static_cast<double>(y)), w(static_cast<double>(w)), h(static_cast<double>(h)) {}

    template <typename Number>
    Rect(const Vec2 &pos, Number w, Number h)
        : x(pos.x), y(pos.y), w(static_cast<double>(w)), h(static_cast<double>(h)) {}

    template <typename Number>
    Rect(Number x, Number y, const Vec2 &size)
        : x(static_cast<double>(x)), y(static_cast<double>(y)), w(size.x), h(size.y) {}

    Rect(const Vec2 &pos, const Vec2 &size)
        : x(pos.x), y(pos.y), w(size.x), h(size.y) {}
};
