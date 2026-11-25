#pragma once

#include "zombie/graphics.h"

namespace zombie {

// 玩家和怪物共用的实体；目前 ActorType 只有 player 和 zombie。
class Entity {
public:
    explicit Entity(ActorType type = ActorType::player);

    void update(double deltaTime);
    void takeDamage(int damage);

private:
    ActorType type_;
    VisualState visualState_ = VisualState::normal;
    SDL_FPoint position_{0, 0}; // 世界坐标。
    SDL_FPoint size_{32, 32};   // 基础尺寸，后续用于绘制及碰撞。
    SDL_FPoint velocity_{0, 0}; // 世界单位／秒。
    double angle_ = 0;         // 朝右为 0 度，顺时针为正。
    float moveSpeed_ = 0;      // TODO: 按玩家／僵尸类型配置移动速度。
    int maxHealth_ = 100;
    int health_ = 100;

    // TODO: 接入输入或怪物追踪、碰撞、攻击及状态持续时间。
};

} // namespace zombie
