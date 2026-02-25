#include "Texture.hpp"

#include "Renderer.hpp"

Texture::Texture(const std::string &filePath)
{
    if (filePath.empty())
        throw std::invalid_argument("File path cannot be empty");

    SDL_Surface *surface = SDL_LoadPNG(filePath.c_str());
    if (!surface)
        throw std::runtime_error("Failed to load image: " + std::string(SDL_GetError()));

    m_texture = SDL_CreateTextureFromSurface(renderer::_get(), surface);
    SDL_DestroySurface(surface);
    if (!m_texture)
        throw std::runtime_error("Failed to load texture: " + std::string(SDL_GetError()));

    float w, h;
    if (!SDL_GetTextureSize(m_texture, &w, &h))
    {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
        throw std::runtime_error("Failed to get texture size: " + std::string(SDL_GetError()));
    }

    m_width = static_cast<int>(w);
    m_height = static_cast<int>(h);
    m_clipArea = {0, 0, m_width, m_height};
}

Texture::~Texture()
{
    if (m_texture)
    {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
}

void Texture::setAlpha(const float alpha) const
{
    SDL_SetTextureAlphaModFloat(m_texture, alpha);
}

float Texture::getAlpha() const
{
    float alphaMod;
    SDL_GetTextureAlphaModFloat(m_texture, &alphaMod);
    return alphaMod;
}
