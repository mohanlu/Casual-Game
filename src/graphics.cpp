#include "zombie/graphics.h"
#include <cmath>
#include <stdexcept>

namespace zombie {
namespace {
void require(bool ok, const char* operation) {
    if (!ok) { throw std::runtime_error(std::string(operation) + ": " + SDL_GetError()); }
}
constexpr std::array<const char*, 10> names = {
    "player.png", "player_hurt.png", "player_dead.png", "zombie.png", "zombie_hurt.png",
    "zombie_dead.png", "ammo.png", "health.png", "ground.png", "wall.png"
};
}

SpriteId actorSprite(ActorType actor, VisualState state) {
    static constexpr SpriteId player[] = {SpriteId::player, SpriteId::playerHurt, SpriteId::playerDead};
    static constexpr SpriteId zombie[] = {SpriteId::zombie, SpriteId::zombieHurt, SpriteId::zombieDead};
    const auto index = static_cast<size_t>(state);
    if (index >= 3) { throw std::invalid_argument("Invalid visual state"); }
    return actor == ActorType::player ? player[index] : zombie[index];
}

void Sprite::draw(SDL_Renderer* renderer, const SDL_FRect& destination, double angle) const {
    if (!texture) { return; }
    if (!std::isfinite(angle)) { throw std::invalid_argument("Invalid sprite angle"); }
    // 角度以度计，绕目标矩形中心顺时针旋转。
    require(SDL_RenderTextureRotated(renderer, texture, &source, &destination,
        angle, nullptr, SDL_FLIP_NONE), "Draw sprite");
}

void TextLabel::set(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color) {
    if (renderer_ == renderer && font_ == font && text_ == text &&
        color_.r == color.r && color_.g == color.g && color_.b == color.b && color_.a == color.a) { return; }
    std::unique_ptr<SDL_Texture, SdlDeleter> next;
    float width = 0, height = 0;
    if (!text.empty()) {
        if (!font || !renderer) { throw std::invalid_argument("Text requires font and renderer"); }
        std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(
            TTF_RenderText_Blended(font, text.c_str(), text.size(), color), SDL_DestroySurface);
        require(surface != nullptr, "Render UTF-8 text");
        next.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));
        require(next != nullptr, "Create text texture");
        width = static_cast<float>(surface->w);
        height = static_cast<float>(surface->h);
    }
    texture_ = std::move(next);
    width_ = width;
    height_ = height;
    text_ = text;
    font_ = font;
    renderer_ = renderer;
    color_ = color;
}

void TextLabel::draw(SDL_Renderer* renderer, float x, float y) const {
    if (!texture_) { return; }
    const SDL_FRect destination{x, y, width_, height_};
    require(SDL_RenderTexture(renderer, texture_.get(), nullptr, &destination), "Draw text");
}

void GameGraphics::load(Resources& resources, SDL_Renderer* renderer) {
    sprites_ = {};
    title_.set(renderer, nullptr, "");
    const bool atlasExists = std::filesystem::exists(resources.root() / "images/atlas.png");
    SDL_Texture* atlas = atlasExists ? resources.texture("images/atlas.png") : nullptr;
    if (atlas) {
        float width, height;
        require(SDL_GetTextureSize(atlas, &width, &height), "Read atlas size");
        if (width != 160 || height != 64) {
            throw std::runtime_error("images/atlas.png must be 160x64 (5 columns x 2 rows of 32x32)");
        }
        require(SDL_SetTextureScaleMode(atlas, SDL_SCALEMODE_NEAREST), "Atlas scale mode");
    }
    int missing = 0;
    for (size_t i = 0; i < names.size(); ++i) {
        auto& sprite = sprites_[i];
        if (atlas) {
            sprite.texture = atlas;
            sprite.source = {static_cast<float>((i % 5) * 32), static_cast<float>((i / 5) * 32), 32, 32};
        } else {
            const std::string path = std::string("images/") + names[i];
            if (!std::filesystem::exists(resources.root() / path)) { ++missing; continue; }
            sprite.texture = resources.texture(path);
            float width, height;
            require(SDL_GetTextureSize(sprite.texture, &width, &height), "Read sprite size");
            if (width != 32 || height != 32) { throw std::runtime_error(path + " must be 32x32"); }
            require(SDL_SetTextureScaleMode(sprite.texture, SDL_SCALEMODE_NEAREST), "Sprite scale mode");
        }
    }
    if (missing) { SDL_Log("Assets: %d sprite files pending in src/images (see docs/ASSETS.md)", missing); }
    if (std::filesystem::exists(resources.root() / "images/ui.ttf")) {
        title_.set(renderer, resources.font("images/ui.ttf", 24), "素材预览 / ASSET PREVIEW");
    } else { SDL_Log("Assets: ui.ttf pending in src/images"); }
}

void GameGraphics::drawPreview(SDL_Renderer* renderer) const {
    title_.draw(renderer, 24, 20);
    for (size_t i = 0; i < sprites_.size(); ++i) {
        const SDL_FRect destination{24.0f + static_cast<float>(i % 5) * 100,
            70.0f + static_cast<float>(i / 5) * 100, 64, 64};
        sprites_[i].draw(renderer, destination);
    }
    // 同一正常角色旋转 90 度的预览；实际瞄准角度在玩家控制阶段接入。
    sprite(SpriteId::player).draw(renderer, {24, 290, 64, 64}, 90);
}

} // namespace zombie
