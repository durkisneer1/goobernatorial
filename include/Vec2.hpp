#pragma once

#include <SDL3/SDL.h>

class Vec2
{
public:
    double x{0.0};
    double y{0.0};

    Vec2() = default;

    template <typename Number>
    Vec2(Number x, Number y)
        : x(static_cast<double>(x)), y(static_cast<double>(y)) {}
};
