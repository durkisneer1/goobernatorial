#include "Events.hpp"

#include <SDL3/SDL.h>
#include "Window.hpp"

namespace events
{
    static bool _shouldQuit = false;

    bool shouldQuit()
    {
        return _shouldQuit;
    }

    std::vector<Event> pollEvents()
    {
        std::vector<Event> result;
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                _shouldQuit = true;
                window::close();
                result.push_back(QuitEvent{EventType::QUIT});
                break;

            case SDL_EVENT_KEY_DOWN:
            {
                result.push_back(KeyEvent{EventType::KEY_DOWN, static_cast<int>(event.key.key)});
                break;
            }

            case SDL_EVENT_KEY_UP:
            {
                result.push_back(KeyEvent{EventType::KEY_UP, static_cast<int>(event.key.key)});
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                result.push_back(MouseEvent{
                    EventType::MOUSE_DOWN,
                    static_cast<float>(event.button.x),
                    static_cast<float>(event.button.y),
                    static_cast<int>(event.button.button)});
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                result.push_back(MouseEvent{
                    EventType::MOUSE_UP,
                    static_cast<float>(event.button.x),
                    static_cast<float>(event.button.y),
                    static_cast<int>(event.button.button)});
                break;
            }

            case SDL_EVENT_MOUSE_MOTION:
            {
                result.push_back(MotionEvent{
                    EventType::MOUSE_MOTION,
                    static_cast<float>(event.motion.x),
                    static_cast<float>(event.motion.y)});
                break;
            }

            default:
                break;
            }
        }

        return result;
    }
}
