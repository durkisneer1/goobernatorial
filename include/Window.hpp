#pragma once

#include <string>

namespace window
{
    void create(const std::string &title, int width, int height);
    bool isOpen();
    void close();

    void _quit();
}
