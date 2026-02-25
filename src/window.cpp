#include "Window.hpp"

#include <SDL3/SDL.h>
#include <stdexcept>

#include "Renderer.hpp"
#include "Time.hpp"

namespace window
{
    static SDL_Window *_window = nullptr;
    static bool _isOpen = false;

    void create(const std::string &title, const int width, const int height)
    {
        if (_window)
            throw std::runtime_error("Window already created");

        _window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_RESIZABLE);
        if (!_window)
            throw std::runtime_error(SDL_GetError());

        _isOpen = true;
        renderer::_init(_window, width, height);
    }

    bool isOpen()
    {
        time::_tick();
        return _isOpen;
    }

    void close()
    {
        _isOpen = false;
    }

    void _quit()
    {
        if (_window)
        {
            SDL_DestroyWindow(_window);
            _window = nullptr;
        }
        _isOpen = false;
    }

    SDL_Window *_get()
    {
        return _window;
    }
}
