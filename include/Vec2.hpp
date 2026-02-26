#pragma once

#include <SDL3/SDL.h>
#include <cmath>

class Vec2
{
public:
    double x{0.0};
    double y{0.0};

    Vec2() = default;

    template <typename Number>
    Vec2(Number x, Number y)
        : x(static_cast<double>(x)), y(static_cast<double>(y)) {}

    // Arithmetic operators
    Vec2 operator+(const Vec2 &other) const { return {x + other.x, y + other.y}; }
    Vec2 operator-(const Vec2 &other) const { return {x - other.x, y - other.y}; }
    Vec2 operator*(const Vec2 &other) const { return {x * other.x, y * other.y}; }
    Vec2 operator*(double scalar) const { return {x * scalar, y * scalar}; }
    Vec2 operator/(double scalar) const { return {x / scalar, y / scalar}; }
    Vec2 operator-() const { return {-x, -y}; }

    // In-place operators
    Vec2 &operator+=(const Vec2 &other) { x += other.x; y += other.y; return *this; }
    Vec2 &operator-=(const Vec2 &other) { x -= other.x; y -= other.y; return *this; }
    Vec2 &operator*=(double scalar) { x *= scalar; y *= scalar; return *this; }
    Vec2 &operator/=(double scalar) { x /= scalar; y /= scalar; return *this; }

    // Comparison
    bool operator==(const Vec2 &other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vec2 &other) const { return !(*this == other); }

    // Utility
    double length() const { return std::sqrt(x * x + y * y); }
    double length_sq() const { return x * x + y * y; }
    Vec2 normalized() const
    {
        double len = length();
        return len > 1e-8 ? Vec2{x / len, y / len} : Vec2{};
    }

    // Allow scalar * Vec2
    friend Vec2 operator*(double scalar, const Vec2 &v) { return {v.x * scalar, v.y * scalar}; }
};
