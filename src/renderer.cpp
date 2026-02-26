#include "Renderer.hpp"

#include <SDL3/SDL.h>

#include "Texture.hpp"
#include "Transform.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static SDL_Renderer *_renderer = nullptr;
static int cached_render_width = 1280;
static int cached_render_height = 720;

constexpr double TO_DEGREES(const double radians)
{
    return radians * (180.0 / M_PI);
}

namespace renderer
{
    void clear(const Color &color)
    {
        // Cache render output size at the start of frame
        SDL_GetCurrentRenderOutputSize(_renderer, &cached_render_width, &cached_render_height);
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

        Vec2 clipSize{clipArea.w, clipArea.h};
        Vec2 dstSize = clipSize * transform.scale;
        Vec2 dstPos = transform.pos - dstSize * anchor;
        Rect dstRect{dstPos.x, dstPos.y, dstSize.x, dstSize.y};

        // int w = cached_render_width;
        // int h = cached_render_height;
        // if (dstRect.x + dstRect.w < 0.0 ||
        //     dstRect.x >= static_cast<double>(w) ||
        //     dstRect.y + dstRect.h < 0.0 ||
        //     dstRect.y >= static_cast<double>(h))
        // {
        //     return;
        // }

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

    void draw_batch_soa(
        const Texture &texture,
        const double *pos_x, const double *pos_y,
        const double *rot,
        const double *scale_x, const double *scale_y,
        size_t count,
        const Vec2 &anchor, const Vec2 &pivot)
    {
        Rect clipArea = texture.getClipArea();
        if (clipArea.w <= 1e-8 || clipArea.h <= 1e-8)
            return;

        SDL_Texture *sdlTex = texture.getSDL();

        const SDL_FRect srcSDLRect{
            static_cast<float>(clipArea.x),
            static_cast<float>(clipArea.y),
            static_cast<float>(clipArea.w),
            static_cast<float>(clipArea.h),
        };

        SDL_FlipMode flipAxis = SDL_FLIP_NONE;
        if (texture.flip.h)
            flipAxis = static_cast<SDL_FlipMode>(flipAxis | SDL_FLIP_HORIZONTAL);
        if (texture.flip.v)
            flipAxis = static_cast<SDL_FlipMode>(flipAxis | SDL_FLIP_VERTICAL);

        const double cw = clipArea.w;
        const double ch = clipArea.h;

        for (size_t i = 0; i < count; ++i)
        {
            double sx = scale_x[i];
            double sy = scale_y[i];

            double dw = cw * sx;
            double dh = ch * sy;
            double dx = pos_x[i] - dw * anchor.x;
            double dy = pos_y[i] - dh * anchor.y;

            const SDL_FRect dstSDLRect{
                static_cast<float>(dx),
                static_cast<float>(dy),
                static_cast<float>(dw),
                static_cast<float>(dh),
            };

            const SDL_FPoint pivotPoint{
                static_cast<float>(dw * pivot.x),
                static_cast<float>(dh * pivot.y),
            };

            SDL_RenderTextureRotated(
                _renderer, sdlTex, &srcSDLRect, &dstSDLRect, TO_DEGREES(rot[i]),
                &pivotPoint, flipAxis);
        }
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
