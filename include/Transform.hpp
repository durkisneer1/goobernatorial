#pragma once

#include "Vec2.hpp"

struct Transform
{
    Vec2 pos;
    double rot{0};
    Vec2 scale{1, 1};
};
