#pragma once

class Color
{
public:
    int r{0};
    int g{0};
    int b{0};
    int a{255};

    Color() = default;
    Color(int r, int g, int b, int a = 255)
        : r(r), g(g), b(b), a(a) {}
};
