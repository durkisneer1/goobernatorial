import goob


class Kraken:
    def __init__(self) -> None:
        goob.init()
        self.w, self.h = 1280, 720
        goob.create_window("Kraken", self.w, self.h)

        self.texture = goob.Texture("rifle.png")
        self.sprites = goob.InkSprites(
            self.texture,
            goob.Rect(0, 0, self.w, self.h),
            "particle.ink",
        )
        self.frame_count = 0

    def run(self) -> None:
        while goob.window_is_open():
            goob.clear()
            events = goob.poll_events()

            for event in events:
                if event.type == goob.EventType.QUIT:
                    goob.close_window()
                elif event.type == goob.EventType.MOUSE_DOWN:
                    if event.button == 1:
                        self.sprites.add(1000, 2.0)
                    elif event.button == 3:
                        self.sprites.remove(1000)

            dt = goob.get_delta() * 60.0
            self.sprites.update(dt)
            self.sprites.render()

            goob.present()

            self.frame_count += 1
            if self.frame_count % 60 == 0:
                print(f"FPS: {goob.get_fps():.0f} | Count: {self.sprites.count()}")

        goob.quit()


if __name__ == "__main__":
    Kraken().run()
