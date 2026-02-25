#pragma once

#include <vector>
#include <cstdint>
#include <variant>

namespace events
{
    enum class EventType : uint8_t
    {
        NONE,
        KEY_DOWN,
        KEY_UP,
        MOUSE_DOWN,
        MOUSE_UP,
        MOUSE_MOTION,
        QUIT
    };

    struct KeyEvent
    {
        EventType type;
        int keycode;
    };

    struct MouseEvent
    {
        EventType type;
        float x;
        float y;
        int button; // 1=left, 2=middle, 3=right
    };

    struct MotionEvent
    {
        EventType type;
        float x;
        float y;
    };

    struct QuitEvent
    {
        EventType type;
    };

    // Union type for all events
    using Event = std::variant<KeyEvent, MouseEvent, MotionEvent, QuitEvent>;

    // Poll events and return them (can be called from Python)
    std::vector<Event> pollEvents();

    // Check if quit was requested
    bool shouldQuit();
}
