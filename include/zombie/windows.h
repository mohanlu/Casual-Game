#pragma once

#include "zombie/fixed_step.h"
#include "zombie/input.h"
#include "zombie/resources.h"
#include "zombie/graphics.h"
#include "zombie/menu.h"

namespace zombie {

class Windows {
public:
    Windows() = default;
    ~Windows();
    Windows(const Windows&) = delete;
    Windows& operator=(const Windows&) = delete;
    // 窗口与界面流程的入口，返回程序退出码。
    int run();
    // 由主线程中的战斗逻辑调用；只允许 Playing -> Victory / Defeat。
    bool finishGame(bool won);
    GameState state() const { return flow_.state(); }

private:
    void initialize();
    void handleEvent(const SDL_Event& event);
    void update(double deltaTime);
    void render();
    void shutdown();
    // 菜单操作与状态切换后的输入／界面刷新。
    void applyAction(MenuAction action);
    void refreshState();

    std::unique_ptr<SDL_Window, SdlDeleter> window_;
    std::unique_ptr<SDL_Renderer, SdlDeleter> renderer_;
    std::unique_ptr<Resources> resources_;
    std::unique_ptr<GameGraphics> graphics_;
    std::unique_ptr<Menu> menu_;
    GameFlow flow_;
    Input input_;
    FixedStep clock_;
    bool sdlInitialized_ = false;
    bool ttfInitialized_ = false;
    bool running_ = false;
    bool focused_ = true;
    bool minimized_ = false;
    bool vsync_ = false;
};

} // namespace zombie
