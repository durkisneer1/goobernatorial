#!/usr/bin/env python3
"""
Simple example: Opens a window, clears and presents the renderer,
waits for quit event.
"""

import goob

goob.init()

# Create a window
goob.create_window("Goobernatorial", 800, 600)

# Main loop
while goob.window_is_open():
    # Poll events
    events = goob.poll_events()

    for event in events:
        if event.type == goob.EventType.QUIT:
            print("Quit event received, closing window")
            goob.close_window()
        elif event.type == goob.EventType.KEY_DOWN:
            print(f"Key down: {event.keycode}")
        elif event.type == goob.EventType.MOUSE_DOWN:
            print(f"Mouse clicked at ({event.x}, {event.y}), button {event.button}")

    # Clear the screen with black
    goob.clear(goob.Color(0, 0, 0, 255))

    # Present the renderer
    goob.present()

print("Window closed")

goob.quit()
