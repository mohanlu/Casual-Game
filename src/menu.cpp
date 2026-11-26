#include "zombie/menu.h"
#include <stdexcept>

namespace zombie {
namespace {
void fill(SDL_Renderer* renderer, SDL_FRect rect, SDL_Color color) {
    if (!SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a) ||
        !SDL_RenderFillRect(renderer, &rect)) { throw std::runtime_error(SDL_GetError()); }
}
void centered(const TextLabel& label, SDL_Renderer* renderer, float y) {
    label.draw(renderer, (960 - label.width()) / 2, y);
}
}

void Menu::rebuild(GameState state, Resources& resources, SDL_Renderer* renderer) {
    state_ = state;
    buttons_.clear();
    selected_ = 0;
    pressed_ = -1;
    const SDL_Color accent = state == GameState::Defeat ? SDL_Color{221, 116, 102, 255} : SDL_Color{210, 192, 128, 255};
    auto* small = resources.font("images/ui.ttf", 18);
    auto* regular = resources.font("images/ui.ttf", 23);
    auto* large = resources.font("images/ui.ttf", 48);
    const char* title = "ZOMBIE GAME";
    const char* subtitle = "废墟之中，继续生存";
    switch (state) {
    case GameState::Playing: title = "场景已就绪"; subtitle = "玩法开发中 · 按 Esc 或空格暂停"; break;
    case GameState::Paused: title = "PAUSED"; subtitle = "游戏已暂停"; break;
    case GameState::Victory: title = "VICTORY"; subtitle = "本局结束"; break;
    case GameState::Defeat: title = "DEFEAT"; subtitle = "本局结束"; break;
    default: break;
    }
    eyebrow_.set(renderer, small, "生存行动  /  DEMO 01", accent);
    title_.set(renderer, large, title, accent);
    subtitle_.set(renderer, regular, subtitle, {186, 194, 187, 255});
    footer_.set(renderer, small, state == GameState::Playing ? "世界与角色逻辑将在后续接入" :
        "鼠标点击选择  ·  ↑↓ / W S 切换  ·  Enter 确认", {139, 150, 145, 255});
    auto add = [&](const char* text, MenuAction action) {
        Button button;
        button.rectangle = {310, 355 + static_cast<float>(buttons_.size()) * 72, 340, 56};
        button.action = action;
        button.label.set(renderer, regular, text);
        buttons_.push_back(std::move(button));
    };
    if (state == GameState::MainMenu) { add("开始游戏", MenuAction::Start); }
    if (state == GameState::Paused) { add("继续游戏", MenuAction::Resume); }
    if (state == GameState::Paused || state == GameState::Victory || state == GameState::Defeat) {
        add("返回主菜单", MenuAction::MainMenu);
    }
    if (state != GameState::Playing) { add("退出游戏", MenuAction::Quit); }
}

int Menu::hit(float x, float y) const {
    for (size_t i = 0; i < buttons_.size(); ++i) {
        const auto& r = buttons_[i].rectangle;
        if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) { return static_cast<int>(i); }
    }
    return -1;
}

MenuAction Menu::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) { pressed_ = -1; }
    if (buttons_.empty()) { return MenuAction::None; }
    if (event.type == SDL_EVENT_MOUSE_MOTION) { selected_ = hit(event.motion.x, event.motion.y); }
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
        pressed_ = hit(event.button.x, event.button.y);
        selected_ = pressed_;
    }
    if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
        const int released = hit(event.button.x, event.button.y);
        const int pressed = pressed_;
        pressed_ = -1;
        if (pressed >= 0 && pressed == released) { return buttons_[pressed].action; }
    }
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        const auto key = event.key.scancode;
        const int count = static_cast<int>(buttons_.size());
        if (key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_S) {
            selected_ = (selected_ + 1) % count;
        } else if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_W) {
            selected_ = selected_ < 0 ? count - 1 : (selected_ + count - 1) % count;
        } else if ((key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER) && selected_ >= 0) {
            return buttons_[selected_].action;
        }
    }
    return MenuAction::None;
}

void Menu::draw(SDL_Renderer* renderer) const {
    if (state_ == GameState::Playing) {
        fill(renderer, {0, 0, 960, 125}, {12, 19, 21, 235});
        centered(title_, renderer, 18);
        centered(subtitle_, renderer, 83);
        fill(renderer, {0, 655, 960, 65}, {12, 19, 21, 235});
        centered(footer_, renderer, 677);
        return;
    }
    fill(renderer, {0, 0, 960, 720}, {8, 13, 15, 190});
    fill(renderer, {230, 140, 500, 455}, {19, 29, 31, 245});
    fill(renderer, {230, 140, 500, 3}, {162, 148, 102, 255});
    centered(eyebrow_, renderer, 173);
    centered(title_, renderer, 220);
    centered(subtitle_, renderer, 294);
    for (size_t i = 0; i < buttons_.size(); ++i) {
        const auto& b = buttons_[i];
        const bool selected = static_cast<int>(i) == selected_;
        fill(renderer, b.rectangle, selected ? SDL_Color{73, 83, 66, 255} : SDL_Color{35, 46, 47, 255});
        if (selected) { fill(renderer, {b.rectangle.x, b.rectangle.y, 4, b.rectangle.h}, {221, 201, 136, 255}); }
        b.label.draw(renderer, b.rectangle.x + (b.rectangle.w - b.label.width()) / 2,
            b.rectangle.y + (b.rectangle.h - b.label.height()) / 2);
    }
    centered(footer_, renderer, 632);
}
} // namespace zombie
