#include "zombie/menu.h"
#include <stdexcept>

namespace zombie {
namespace {
void fill(SDL_Renderer* renderer, SDL_FRect rect, SDL_Color color) {
    if (!SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a) ||
        !SDL_RenderFillRect(renderer, &rect)) { throw std::runtime_error(SDL_GetError()); }
}
void centered(const TextLabel& label, SDL_Renderer* renderer, float y, float width, float offsetY) {
    label.draw(renderer, (width - label.width()) / 2, y + offsetY);
}
}

void Menu::rebuild(GameState state, Resources& resources, SDL_Renderer* renderer,
    const Settings& settings, bool settingsOpen, const char* status, bool controlsOpen) {
    const float targetWidth = width_, targetHeight = height_;
    width_ = 960; height_ = 720;
    settingsOpen_ = settingsOpen;
    controlsOpen_ = settingsOpen && controlsOpen;
    controlKeys_.clear();
    controlDescriptions_.clear();
    state_ = state;
    buttons_.clear();
    selected_ = -1;
    pressed_ = -1;
    const SDL_Color accent = state == GameState::Defeat ? SDL_Color{221, 116, 102, 255} : SDL_Color{210, 192, 128, 255};
    auto* small = resources.font("images/ui.ttf", 18);
    auto* regular = resources.font("images/ui.ttf", 23);
    auto* large = resources.font("images/ui.ttf", 48);
    const char* title = "ZOMBIE GAME";
    const char* subtitle = "废墟之中，继续生存";
    switch (state) {
    case GameState::Playing: title = "场景已就绪"; subtitle = "玩法开发中 · 按 Esc / 空格 / P 暂停"; break;
    case GameState::Paused: title = "PAUSED"; subtitle = "游戏已暂停"; break;
    case GameState::Victory: title = "VICTORY"; subtitle = "本局结束"; break;
    case GameState::Defeat: title = "DEFEAT"; subtitle = "本局结束"; break;
    default: break;
    }
    if (settingsOpen_) { title = "设置"; }
    if (controlsOpen_) { title = "键位说明"; subtitle = ""; }
    eyebrow_.set(renderer, small, "生存行动  /  DEMO 01", accent);
    title_.set(renderer, large, title, accent);
    subtitle_.set(renderer, regular, subtitle, {186, 194, 187, 255});
    footer_.set(renderer, small, state == GameState::Playing ? "世界与角色逻辑将在后续接入" : "",
        {139, 150, 145, 255});
    auto add = [&](const char* text, MenuAction action) {
        Button button;
        button.rectangle = {310, 355 + static_cast<float>(buttons_.size()) * 72, 340, 56};
        button.action = action;
        button.label.set(renderer, regular, text);
        buttons_.push_back(std::move(button));
    };
    if (controlsOpen_) {
        auto binding = [&](const char* key, const char* description) {
            TextLabel keyLabel, descriptionLabel;
            keyLabel.set(renderer, small, key, {210, 192, 128, 255});
            descriptionLabel.set(renderer, small, description, {186, 194, 187, 255});
            controlKeys_.push_back(std::move(keyLabel));
            controlDescriptions_.push_back(std::move(descriptionLabel));
        };
        binding("Esc / 空格 / P", "暂停 / 继续游戏");
        binding("鼠标左键（按住）", "点击/单发射击(连发)");
        binding("Esc", "返回上一级");
        binding("W", "向上移动");
        binding("S", "向下移动");
        binding("A", "向左移动");
        binding("D", "向右移动");
        binding("鼠标移动", "瞄准");
        add("", MenuAction::Back);
        buttons_.back().rectangle = {24, 24, 48, 48};
        buttons_.back().icon = resources.texture("images/back.png");
        setSize(targetWidth, targetHeight);
        return;
    }
    if (settingsOpen_) {
        const bool canChangeZombies = state == GameState::MainMenu;
        zombieLabel_.set(renderer, regular, canChangeZombies ? "僵尸数量" : "僵尸数量（本局已锁定）", {210, 192, 128, 255});
        displayLabel_.set(renderer, regular, "显示设置", {210, 192, 128, 255});
        status_.set(renderer, small, status, {221, 192, 128, 255});
        auto option = [&](const char* text, MenuAction action, SDL_FRect rect, bool checked, bool enabled = true) {
            add(text, action);
            auto& button = buttons_.back();
            button.rectangle = rect;
            button.option = true;
            button.checked = checked;
            button.enabled = enabled;
            if (!enabled) { button.label.set(renderer, regular, text, {108, 118, 115, 255}); }
        };
        option("50", MenuAction::Zombies50, {300, 362, 112, 48}, settings.zombieCount == 50, canChangeZombies);
        option("100", MenuAction::Zombies100, {424, 362, 112, 48}, settings.zombieCount == 100, canChangeZombies);
        option("200", MenuAction::Zombies200, {548, 362, 112, 48}, settings.zombieCount == 200, canChangeZombies);
        option("小窗", MenuAction::Windowed, {300, 462, 174, 48}, !settings.fullscreen);
        option("全屏", MenuAction::Fullscreen, {486, 462, 174, 48}, settings.fullscreen);
        add("键位", MenuAction::Controls);
        buttons_.back().rectangle = {306, 548, 348, 48};
        add("", MenuAction::Back);
        buttons_.back().rectangle = {24, 24, 48, 48};
        buttons_.back().icon = resources.texture("images/back.png");
        setSize(targetWidth, targetHeight);
        return;
    }
    if (state == GameState::MainMenu) {
        add("开始游戏", MenuAction::Start);
        add("设置", MenuAction::Settings);
    }
    if (state == GameState::Paused) {
        add("继续游戏", MenuAction::Resume);
        add("设置", MenuAction::Settings);
    }
    if (state == GameState::Paused || state == GameState::Victory || state == GameState::Defeat) {
        add("返回主菜单", MenuAction::MainMenu);
    }
    if (state != GameState::Playing) { add("退出游戏", MenuAction::Quit); }
    if (state == GameState::Paused) {
        for (size_t i = 0; i < buttons_.size(); ++i) {
            buttons_[i].rectangle = {310, 345 + static_cast<float>(i) * 60, 340, 48};
        }
    }
    setSize(targetWidth, targetHeight);
}

void Menu::setSize(float width, float height) {
    for (auto& button : buttons_) {
        if (button.icon) { continue; } // 返回图标固定在左上角。
        button.rectangle.x += (width - width_) / 2;
        button.rectangle.y += (height - height_) / 2;
    }
    if (width != width_ || height != height_) { pressed_ = -1; selected_ = -1; }
    width_ = width; height_ = height;
}

int Menu::hit(float x, float y) const {
    for (size_t i = 0; i < buttons_.size(); ++i) {
        if (!buttons_[i].enabled) { continue; }
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
    return MenuAction::None;
}

void Menu::draw(SDL_Renderer* renderer) const {
    const float offsetX = (width_ - 960) / 2, offsetY = (height_ - 720) / 2;
    if (state_ == GameState::Playing && !settingsOpen_) {
        fill(renderer, {0, 0, width_, 125}, {12, 19, 21, 235});
        centered(title_, renderer, 18, width_, 0);
        centered(subtitle_, renderer, 83, width_, 0);
        fill(renderer, {0, height_ - 65, width_, 65}, {12, 19, 21, 235});
        centered(footer_, renderer, height_ - 43, width_, 0);
        return;
    }
    // 所有菜单直接覆盖整个逻辑画面，不透出地面或保留浮层边框。
    fill(renderer, {0, 0, width_, height_}, {19, 29, 31, 255});
    centered(eyebrow_, renderer, 173, width_, offsetY);
    centered(title_, renderer, 220, width_, offsetY);
    centered(subtitle_, renderer, 294, width_, offsetY);
    if (controlsOpen_) {
        for (size_t i = 0; i < controlKeys_.size(); ++i) {
            const float y = 322 + static_cast<float>(i) * 34;
            controlKeys_[i].draw(renderer, 270 + offsetX, y + offsetY);
            controlDescriptions_[i].draw(renderer, 450 + offsetX, y + offsetY);
        }
    } else if (settingsOpen_) {
        zombieLabel_.draw(renderer, 300 + offsetX, 327 + offsetY);
        displayLabel_.draw(renderer, 300 + offsetX, 427 + offsetY);
        centered(status_, renderer, 516, width_, offsetY);
    }
    for (size_t i = 0; i < buttons_.size(); ++i) {
        const auto& b = buttons_[i];
        const bool selected = static_cast<int>(i) == selected_;
        if (b.icon) {
            if (selected) { fill(renderer, b.rectangle, {73, 83, 66, 255}); }
            const SDL_FRect destination{b.rectangle.x + 8, b.rectangle.y + 8, 32, 32};
            const SDL_Color color = selected ? SDL_Color{235, 218, 160, 255} : SDL_Color{210, 192, 128, 255};
            if (!SDL_SetTextureColorMod(b.icon, color.r, color.g, color.b) ||
                !SDL_RenderTexture(renderer, b.icon, nullptr, &destination)) {
                throw std::runtime_error(SDL_GetError());
            }
            continue;
        }
        fill(renderer, b.rectangle, !b.enabled ? SDL_Color{27, 36, 37, 255} :
            selected ? SDL_Color{73, 83, 66, 255} : SDL_Color{35, 46, 47, 255});
        if (selected) { fill(renderer, {b.rectangle.x, b.rectangle.y, 4, b.rectangle.h}, {221, 201, 136, 255}); }
        if (b.option) {
            const SDL_FRect box{b.rectangle.x + 12, b.rectangle.y + 14, 20, 20};
            fill(renderer, box, b.enabled ? SDL_Color{162, 170, 155, 255} : SDL_Color{83, 93, 90, 255});
            fill(renderer, {box.x + 2, box.y + 2, 16, 16}, {19, 29, 31, 255});
            if (b.checked) { fill(renderer, {box.x + 5, box.y + 5, 10, 10},
                b.enabled ? SDL_Color{221, 201, 136, 255} : SDL_Color{108, 118, 115, 255}); }
        }
        b.label.draw(renderer, b.rectangle.x + (b.rectangle.w - b.label.width()) / 2 + (b.option ? 14 : 0),
            b.rectangle.y + (b.rectangle.h - b.label.height()) / 2);
    }
    centered(footer_, renderer, 632, width_, offsetY);
}
} // namespace zombie
