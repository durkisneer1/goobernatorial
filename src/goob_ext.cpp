#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/variant.h>

#include "Events.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"
#include "Time.hpp"
#include "Vec2.hpp"
#include "Color.hpp"
#include "Transform.hpp"
#include "Rect.hpp"

namespace nb = nanobind;
using namespace nb::literals;

void init()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        throw std::runtime_error("SDL failed to initialize: " + std::string(SDL_GetError()));
}

void quit()
{
    // Clean up in reverse order: renderer -> window -> SDL
    renderer::_quit();
    window::_quit();
    if (SDL_WasInit(0))
        SDL_Quit();
}

NB_MODULE(goob, m)
{
    // ========== Event System ==========
    nb::enum_<events::EventType>(m, "EventType")
        .value("NONE", events::EventType::NONE)
        .value("KEY_DOWN", events::EventType::KEY_DOWN)
        .value("KEY_UP", events::EventType::KEY_UP)
        .value("MOUSE_DOWN", events::EventType::MOUSE_DOWN)
        .value("MOUSE_UP", events::EventType::MOUSE_UP)
        .value("MOUSE_MOTION", events::EventType::MOUSE_MOTION)
        .value("QUIT", events::EventType::QUIT);

    nb::class_<events::KeyEvent>(m, "KeyEvent")
        .def_rw("type", &events::KeyEvent::type)
        .def_rw("keycode", &events::KeyEvent::keycode);

    nb::class_<events::MouseEvent>(m, "MouseEvent")
        .def_rw("type", &events::MouseEvent::type)
        .def_rw("x", &events::MouseEvent::x)
        .def_rw("y", &events::MouseEvent::y)
        .def_rw("button", &events::MouseEvent::button);

    nb::class_<events::MotionEvent>(m, "MotionEvent")
        .def_rw("type", &events::MotionEvent::type)
        .def_rw("x", &events::MotionEvent::x)
        .def_rw("y", &events::MotionEvent::y);

    nb::class_<events::QuitEvent>(m, "QuitEvent")
        .def_rw("type", &events::QuitEvent::type);

    m.def("poll_events", &events::pollEvents, "Poll SDL events and return list of event objects");
    m.def("should_quit", &events::shouldQuit, "Check if quit was requested");

    // ========== Vec2 ==========
    nb::class_<Vec2>(m, "Vec2")
        .def(nb::init<>())
        .def(nb::init<double, double>())
        .def_rw("x", &Vec2::x)
        .def_rw("y", &Vec2::y);

    // ========== Color ==========
    nb::class_<Color>(m, "Color")
        .def(nb::init<>())
        .def(nb::init<int, int, int>())
        .def(nb::init<int, int, int, int>())
        .def_rw("r", &Color::r)
        .def_rw("g", &Color::g)
        .def_rw("b", &Color::b)
        .def_rw("a", &Color::a);

    // ========== Rect ==========
    nb::class_<Rect>(m, "Rect")
        .def(nb::init<>())
        .def(nb::init<double, double, double, double>())
        .def(nb::init<const Vec2 &, double, double>())
        .def(nb::init<double, double, const Vec2 &>())
        .def(nb::init<const Vec2 &, const Vec2 &>())
        .def_rw("x", &Rect::x)
        .def_rw("y", &Rect::y)
        .def_rw("w", &Rect::w)
        .def_rw("h", &Rect::h);

    // ========== Transform ==========
    nb::class_<Transform>(m, "Transform")
        .def(nb::init<>())
        .def_rw("pos", &Transform::pos)
        .def_rw("rot", &Transform::rot)
        .def_rw("scale", &Transform::scale);

    // ========== Texture ==========
    nb::class_<Texture>(m, "Texture")
        .def(nb::init<const std::string &>())
        .def("get_width", &Texture::getWidth)
        .def("get_height", &Texture::getHeight)
        .def("get_size", &Texture::getSize)
        .def("get_clip_area", &Texture::getClipArea)
        .def("set_clip_area", &Texture::setClipArea)
        .def("get_alpha", &Texture::getAlpha)
        .def("set_alpha", &Texture::setAlpha)
        .def_rw("flip", &Texture::flip);

    nb::class_<Texture::Flip>(m, "TextureFlip")
        .def(nb::init<>())
        .def_rw("h", &Texture::Flip::h)
        .def_rw("v", &Texture::Flip::v);

    // ========== Window ==========
    m.def("create_window", &window::create, "title"_a, "width"_a, "height"_a);
    m.def("window_is_open", &window::isOpen);
    m.def("close_window", &window::close);

    // ========== Renderer ==========
    m.def("clear", nb::overload_cast<const Color &>(&renderer::clear), "color"_a = Color{0, 0, 0, 255});
    m.def("present", &renderer::present);
    m.def("draw", &renderer::draw, "texture"_a, "transform"_a, "anchor"_a = Vec2{}, "pivot"_a = Vec2{});

    // ========== Time ==========
    m.def("get_delta", &time::getDelta);
    m.def("get_fps", &time::getFPS);

    // ========== Cleanup ==========
    m.def("init", &init, "Initialize goob");
    m.def("quit", &quit, "Quit goob and clean up all subsystems");
}
