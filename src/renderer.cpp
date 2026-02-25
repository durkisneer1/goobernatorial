#include "Renderer.hpp"

#include <SDL3/SDL.h>

#include "Texture.hpp"
#include "Transform.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static SDL_Renderer *_renderer = nullptr;

constexpr double TO_DEGREES(const double radians)
{
    return radians * (180.0 / M_PI);
}

namespace renderer
{
    void clear(const Color &color)
    {
        SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
        SDL_RenderClear(_renderer);
    }

    void present()
    {
        SDL_RenderPresent(_renderer);
    }

    void draw(const Texture &texture, const Transform &transform, const Vec2 &anchor, const Vec2 &pivot)
    {
        Rect clipArea = texture.getClipArea();
        if (clipArea.w <= 1e-8 || clipArea.h <= 1e-8)
            return;

        if (std::abs(transform.scale.x) < 1e-8 && std::abs(transform.scale.y) < 1e-8)
            return;
        if (texture.getAlpha() == 0.0f)
            return;

        Rect dstRect{
            0.0,
            0.0,
            clipArea.w * transform.scale.x,
            clipArea.h * transform.scale.y,
        };

        dstRect.x = transform.pos.x - (dstRect.w * anchor.x);
        dstRect.y = transform.pos.y - (dstRect.h * anchor.y);

        const SDL_FRect dstSDLRect{
            static_cast<float>(dstRect.x),
            static_cast<float>(dstRect.y),
            static_cast<float>(dstRect.w),
            static_cast<float>(dstRect.h),
        };
        const SDL_FRect srcSDLRect{
            static_cast<float>(clipArea.x),
            static_cast<float>(clipArea.y),
            static_cast<float>(clipArea.w),
            static_cast<float>(clipArea.h),
        };

        // Pivot is normalized 0..1 relative to dstRect, for rotation center
        const SDL_FPoint pivotPoint{
            static_cast<float>(dstRect.w * pivot.x),
            static_cast<float>(dstRect.h * pivot.y),
        };

        SDL_FlipMode flipAxis = SDL_FLIP_NONE;
        if (texture.flip.h)
            flipAxis = static_cast<SDL_FlipMode>(flipAxis | SDL_FLIP_HORIZONTAL);
        if (texture.flip.v)
            flipAxis = static_cast<SDL_FlipMode>(flipAxis | SDL_FLIP_VERTICAL);

        SDL_RenderTextureRotated(
            _renderer, texture.getSDL(), &srcSDLRect, &dstSDLRect, TO_DEGREES(transform.rot),
            &pivotPoint, flipAxis);
    }

    void _init(SDL_Window *window, const int width, const int height)
    {
        _renderer = SDL_CreateGPURenderer(nullptr, window);
        if (_renderer == nullptr)
            throw std::runtime_error("Renderer failed to create: " + std::string(SDL_GetError()));

        SDL_SetRenderLogicalPresentation(_renderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
        SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
    }

    void _quit()
    {
        if (_renderer)
        {
            SDL_DestroyRenderer(_renderer);
            _renderer = nullptr;
        }
    }

    SDL_Renderer *_get()
    {
        return _renderer;
    }
}
