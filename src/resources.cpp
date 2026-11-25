#include "zombie/resources.h"
#include <SDL3_image/SDL_image.h>
#include <cmath>
#include <stdexcept>

namespace zombie {

Resources::Resources(SDL_Renderer* renderer) : renderer_(renderer) {
    const char* base = SDL_GetBasePath();
    if (!renderer_ || !base) { throw std::runtime_error("Resource initialization failed"); }
    root_ = std::filesystem::u8path(base) / "assets";
}

SDL_Texture* Resources::texture(const std::string& relativePath) {
    auto found = textures_.find(relativePath);
    if (found != textures_.end()) { return found->second.get(); }
    const auto path = (root_ / std::filesystem::u8path(relativePath)).u8string();
    std::unique_ptr<SDL_Texture, SdlDeleter> result(IMG_LoadTexture(renderer_, path.c_str()));
    if (!result) { throw std::runtime_error("Texture " + path + ": " + SDL_GetError()); }
    auto* pointer = result.get();
    textures_.emplace(relativePath, std::move(result));
    return pointer;
}

TTF_Font* Resources::font(const std::string& relativePath, float pointSize) {
    if (!std::isfinite(pointSize) || pointSize <= 0) { throw std::invalid_argument("Invalid font size"); }
    const auto key = std::make_pair(relativePath, pointSize);
    auto found = fonts_.find(key);
    if (found != fonts_.end()) { return found->second.get(); }
    const auto path = (root_ / std::filesystem::u8path(relativePath)).u8string();
    std::unique_ptr<TTF_Font, SdlDeleter> result(TTF_OpenFont(path.c_str(), pointSize));
    if (!result) { throw std::runtime_error("Font " + path + ": " + SDL_GetError()); }
    auto* pointer = result.get();
    fonts_.emplace(key, std::move(result));
    return pointer;
}

void Resources::clear() {
    fonts_.clear();
    textures_.clear();
}

} // namespace zombie
