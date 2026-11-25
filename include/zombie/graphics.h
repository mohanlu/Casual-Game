#pragma once

#include "zombie/resources.h"
#include <array>

namespace zombie {

enum class SpriteId { player, playerHurt, playerDead, zombie, zombieHurt, zombieDead,
    ammo, health, ground, wall, count };
enum class ActorType { player, zombie };
enum class VisualState { normal, hurt, dead };
SpriteId actorSprite(ActorType actor, VisualState state);

struct Sprite {
    SDL_Texture* texture = nullptr; // 由 Resources 持有。
    SDL_FRect source{0, 0, 32, 32};
    void draw(SDL_Renderer* renderer, const SDL_FRect& destination, double angle = 0) const;
};

class TextLabel {
public:
    void set(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
        SDL_Color color = {235, 235, 235, 255});
    void draw(SDL_Renderer* renderer, float x, float y) const;
    float width() const { return width_; }
    float height() const { return height_; }

private:
    std::unique_ptr<SDL_Texture, SdlDeleter> texture_;
    std::string text_;
    TTF_Font* font_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Color color_{};
    float width_ = 0;
    float height_ = 0;
};

class GameGraphics {
public:
    // 存在 atlas.png 时使用图集，否则逐个加载独立图片；缺图保留空位。
    void load(Resources& resources, SDL_Renderer* renderer);
    const Sprite& sprite(SpriteId id) const { return sprites_.at(static_cast<size_t>(id)); }
    void drawPreview(SDL_Renderer* renderer) const;

private:
    std::array<Sprite, static_cast<size_t>(SpriteId::count)> sprites_{};
    TextLabel title_;
};

} // namespace zombie
