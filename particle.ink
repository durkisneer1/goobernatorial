@behavior particle:
    if pos.y <= bounds.y or pos.y + rect_h >= bounds.y + bounds.h:
        dir.y *= -1
    elif pos.x <= bounds.x or pos.x + rect_w >= bounds.x + bounds.w:
        dir.x *= -1

    rot = (rot + angle_speed * 0.1 * dt) % (2 * PI)

    pos.x += dir.x * speed * dt
    pos.y += dir.y * speed * dt
