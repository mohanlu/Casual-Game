#pragma once

#include <SDL3/SDL.h>

namespace zombie {

class Entity;

enum class ItemType { ammo, health };

// 弹药补给和回血物品共用的道具类。
class Item {
public:
    explicit Item(ItemType type = ItemType::ammo, int amount = 20);

    void applyTo(Entity& entity);

private:
    ItemType type_;
    SDL_FPoint position_{0, 0}; // 世界坐标。
    SDL_FPoint size_{32, 32};
    int amount_;              // 补给弹药数或恢复生命值。
    bool consumed_ = false;   // 是否已被拾取并消耗。

    // TODO: 接入生成、碰撞拾取及道具图片选择。
};

} // namespace zombie
