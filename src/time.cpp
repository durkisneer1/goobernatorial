#include "Time.hpp"

#include <SDL3/SDL.h>

namespace time
{
    static uint64_t _lastTick = 0;
    static double _fps = 0.0;
    static double _delta = 0.0;

    double getDelta()
    {
        return _delta;
    }

    double getFPS()
    {
        return _fps;
    }

    void _tick()
    {
        uint64_t now = SDL_GetTicksNS();

        if (_lastTick == 0)
        {
            _lastTick = now;
            _delta = 0.0;
            _fps = 0;
            return;
        }

        uint64_t frameTime = now - _lastTick;
        _lastTick = now;
        _delta = static_cast<double>(frameTime) / SDL_NS_PER_SECOND;
        _fps = _delta > 0.0 ? 1.0 / _delta : 0.0;
    }
}
