#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <stdexcept>

namespace zombie {
struct ViewportSize { float width, height; };
inline ViewportSize fitViewport(SDL_Renderer* renderer) {
    int width = 0, height = 0;
    if (!SDL_GetRenderOutputSize(renderer, &width, &height)) { throw std::runtime_error(SDL_GetError()); }
    const float scale = std::max(0.001f, std::min(width / 960.0f, height / 720.0f));
    // 扩展可用画布，文字和图形使用相同的横纵缩放，不留黑边。
    if (!SDL_SetRenderLogicalPresentation(renderer, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED) ||
        !SDL_SetRenderScale(renderer, scale, scale)) { throw std::runtime_error(SDL_GetError()); }
    return {width / scale, height / scale};
}
} // namespace zombie
