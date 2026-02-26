#include "InkSprites.hpp"

#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

#include "ink/Lexer.hpp"
#include "ink/Parser.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

InkSprites::InkSprites(Texture *texture, const Rect &bounds, const std::string &scriptPath)
    : m_texture(texture), m_bounds(bounds)
{
    // Read the .ink script
    std::ifstream file(scriptPath);
    if (!file.is_open())
    {
        throw std::runtime_error("Ink: could not open script '" + scriptPath + "'");
    }

    std::stringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();

    // Lex + parse (done once at construction)
    ink::Lexer lexer(source);
    auto tokens = lexer.tokenize();

    ink::Parser parser(tokens);
    m_behavior = parser.parse();
}

void InkSprites::rebindFields()
{
    // Re-bind pointers every frame because vector reallocation
    // from add()/remove() can invalidate them.
    m_interpreter.bindField("pos.x", m_pos_x.data());
    m_interpreter.bindField("pos.y", m_pos_y.data());
    m_interpreter.bindField("dir.x", m_dir_x.data());
    m_interpreter.bindField("dir.y", m_dir_y.data());
    m_interpreter.bindField("rot", m_rot.data());
    m_interpreter.bindField("scale.x", m_scale_x.data());
    m_interpreter.bindField("scale.y", m_scale_y.data());
    m_interpreter.bindField("speed", m_speed.data());
    m_interpreter.bindField("angle_speed", m_angle_speed.data());
    m_interpreter.setCount(m_size);
}

void InkSprites::add(int count, double scale)
{
    std::uniform_real_distribution<double> distX(m_bounds.x, m_bounds.x + m_bounds.w);
    std::uniform_real_distribution<double> distY(m_bounds.y, m_bounds.y + m_bounds.h);
    std::uniform_real_distribution<double> distDir(-1.0, 1.0);
    std::uniform_real_distribution<double> distAngle(0.2, 3.5);
    std::uniform_real_distribution<double> distSpeed(1.0, 7.0);

    size_t newSize = m_size + count;
    m_pos_x.reserve(newSize);
    m_pos_y.reserve(newSize);
    m_dir_x.reserve(newSize);
    m_dir_y.reserve(newSize);
    m_rot.reserve(newSize);
    m_scale_x.reserve(newSize);
    m_scale_y.reserve(newSize);
    m_speed.reserve(newSize);
    m_angle_speed.reserve(newSize);

    for (int i = 0; i < count; i++)
    {
        m_pos_x.push_back(distX(m_rng));
        m_pos_y.push_back(distY(m_rng));

        double dx = distDir(m_rng);
        double dy = distDir(m_rng);
        double len = std::sqrt(dx * dx + dy * dy);
        if (len < 1e-8)
        {
            dx = 1.0;
            dy = 0.0;
            len = 1.0;
        }
        m_dir_x.push_back(dx / len);
        m_dir_y.push_back(dy / len);

        m_rot.push_back(0.0);
        m_scale_x.push_back(scale);
        m_scale_y.push_back(scale);
        m_speed.push_back(distSpeed(m_rng));
        m_angle_speed.push_back(distAngle(m_rng));
    }

    m_size = newSize;
}

void InkSprites::remove(int count)
{
    int toRemove = std::min(count, static_cast<int>(m_size));
    m_size -= toRemove;
    m_pos_x.resize(m_size);
    m_pos_y.resize(m_size);
    m_dir_x.resize(m_size);
    m_dir_y.resize(m_size);
    m_rot.resize(m_size);
    m_scale_x.resize(m_size);
    m_scale_y.resize(m_size);
    m_speed.resize(m_size);
    m_angle_speed.resize(m_size);
}

void InkSprites::update(double dt)
{
    if (m_size == 0)
        return;

    rebindFields();

    // Per-frame constants
    m_interpreter.setConstant("dt", dt);
    m_interpreter.setConstant("bounds.x", m_bounds.x);
    m_interpreter.setConstant("bounds.y", m_bounds.y);
    m_interpreter.setConstant("bounds.w", m_bounds.w);
    m_interpreter.setConstant("bounds.h", m_bounds.h);

    Vec2 texSize = m_texture->getSize();
    m_interpreter.setConstant("rect_w", texSize.x * m_scale_x[0]);
    m_interpreter.setConstant("rect_h", texSize.y * m_scale_y[0]);

    m_interpreter.setConstant("PI", M_PI);

    // Run the behavior script
    m_interpreter.execute(m_behavior);
}

void InkSprites::render(const Vec2 &anchor, const Vec2 &pivot)
{
    if (m_size == 0)
        return;

    renderer::draw_batch_soa(
        *m_texture,
        m_pos_x.data(), m_pos_y.data(),
        m_rot.data(),
        m_scale_x.data(), m_scale_y.data(),
        m_size,
        anchor, pivot);
}
