#pragma once

#include <vector>
#include <random>
#include <string>

#include "Vec2.hpp"
#include "Rect.hpp"
#include "ink/AST.hpp"
#include "ink/Interpreter.hpp"

class Texture;

/// A batch of sprites whose behavior is defined by an Ink script.
///
/// Uses Struct-of-Arrays storage for maximum throughput. The Ink interpreter
/// executes vectorized operations over all sprites each frame — no per-sprite
/// Python callbacks needed.
///
/// Built-in mutable fields (accessible in .ink scripts):
///   pos.x, pos.y      — position
///   dir.x, dir.y      — normalized direction
///   rot                — rotation in radians
///   scale.x, scale.y   — scale factors
///   speed              — movement speed
///   angle_speed        — rotation speed
///
/// Built-in read-only constants:
///   dt             — delta time (set each frame)
///   bounds.x/y/w/h — viewport bounds
///   rect_w, rect_h — scaled sprite dimensions
///   PI             — 3.14159...
class InkSprites
{
public:
    InkSprites(Texture *texture, const Rect &bounds, const std::string &scriptPath);

    void add(int count, double scale = 1.0);
    void remove(int count = 1);
    size_t count() const { return m_size; }

    void update(double dt);
    void render(const Vec2 &anchor = {}, const Vec2 &pivot = {});

private:
    void rebindFields();

    // SoA arrays
    std::vector<double> m_pos_x, m_pos_y;
    std::vector<double> m_dir_x, m_dir_y;
    std::vector<double> m_rot;
    std::vector<double> m_scale_x, m_scale_y;
    std::vector<double> m_speed;
    std::vector<double> m_angle_speed;

    size_t m_size{0};
    Texture *m_texture;
    Rect m_bounds;

    // Ink scripting
    ink::BehaviorDecl m_behavior;
    ink::Interpreter m_interpreter;

    std::mt19937 m_rng{std::random_device{}()};
};
