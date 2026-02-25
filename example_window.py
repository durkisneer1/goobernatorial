import pykraken

pykraken.init(debug=True)
pykraken.window.create("name", 800, 800)
quit()

import math
import random

import goob


class Sprite:
    def __init__(self, texture: goob.Texture, w: int, h: int, scale: float = 1.0) -> None:
        self.position = goob.Vec2(random.uniform(0, w), random.uniform(0, h))

        dx = random.uniform(-1.0, 1.0)
        dy = random.uniform(-1.0, 1.0)
        mag = math.hypot(dx, dy) or 1.0
        self.direction = goob.Vec2(dx / mag, dy / mag)

        self.base_texture = texture
        tw, th = self.base_texture.get_size().x, self.base_texture.get_size().y
        self.rect = goob.Rect(self.position.x, self.position.y, tw * scale, th * scale)

        self.transform = goob.Transform()
        self.transform.pos = goob.Vec2(self.position.x, self.position.y)
        self.transform.rot = 0.0
        self.transform.scale = goob.Vec2(scale, scale)

        self.angle_change = random.uniform(0.2, 3.5)
        self.speed = random.uniform(1.0, 7.0)

    def update(self, dt: float, bg_rect: goob.Rect) -> None:
        if self.rect.y <= bg_rect.y or self.rect.y + self.rect.h >= bg_rect.y + bg_rect.h:
            self.direction.y *= -1.0
        elif self.rect.x <= bg_rect.x or self.rect.x + self.rect.w >= bg_rect.x + bg_rect.w:
            self.direction.x *= -1.0

        self.transform.rot = (self.transform.rot + self.angle_change * dt) % (math.pi * 2.0)

        vx = dt * self.speed * self.direction.x
        vy = dt * self.speed * self.direction.y
        self.transform.pos.x += vx
        self.transform.pos.y += vy

        self.rect = goob.Rect(self.transform.pos.x, self.transform.pos.y, self.rect.w, self.rect.h)

    def render(self) -> None:
        goob.draw(self.base_texture, self.transform)


class Kraken:
    def __init__(self) -> None:
        goob.init()
        self.w, self.h = 1280, 720
        goob.create_window("Kraken", self.w, self.h)

        self.bg_rect = goob.Rect(0, 0, self.w, self.h)
        self.texture = goob.Texture("IMG_3325.png")
        self.sprites: list[Sprite] = []
        self.dt = 1.0
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
                        for _ in range(100):
                            self.sprites.append(Sprite(self.texture, self.w, self.h, 2.0))
                    elif event.button == 3 and self.sprites:
                        self.sprites.pop()

            for sprite in self.sprites:
                sprite.update(self.dt, self.bg_rect)
                sprite.render()

            self.dt = goob.get_delta() * 60.0
            goob.present()

            self.frame_count += 1
            if self.frame_count % 60 == 0:
                print(f"FPS: {goob.get_fps():.0f} | Count: {len(self.sprites)}")

        goob.quit()


if __name__ == "__main__":
    Kraken().run()
