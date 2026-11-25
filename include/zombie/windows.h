#pragma once

#include "zombie/fixed_step.h"
#include "zombie/input.h"
#include "zombie/resources.h"
#include "zombie/graphics.h"

namespace zombie {

class Windows {
public:
    Windows() = default;
    ~Windows();
    Windows(const Windows&) = delete;
    Windows& operator=(const Windows&) = delete;
    // 窗口与界面流程的入口，返回程序退出码。
    int run();

private:
    void initialize();
    void handleEvent(const SDL_Event& event);
    void update(double deltaTime);
    void render();
    void shutdown();
    // 开始菜单界面。
    void showMainMenu();

    std::unique_ptr<SDL_Window, SdlDeleter> window_;
    std::unique_ptr<SDL_Renderer, SdlDeleter> renderer_;
    std::unique_ptr<Resources> resources_;
    std::unique_ptr<GameGraphics> graphics_;
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
