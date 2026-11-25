#include "zombie/windows.h"
#include <stdexcept>

namespace zombie {

namespace {
void require(bool success, const char* action) {
    if (!success) { throw std::runtime_error(std::string(action) + ": " + SDL_GetError()); }
}
}

Windows::~Windows() { shutdown(); }

void Windows::initialize() {
    sdlInitialized_ = true;
    require(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS), "SDL_Init");
    require(TTF_Init(), "TTF_Init");
    ttfInitialized_ = true;
    window_.reset(SDL_CreateWindow("Zombie Game", 960, 720,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY));
    require(window_ != nullptr, "SDL_CreateWindow");
    renderer_.reset(SDL_CreateRenderer(window_.get(), nullptr));
    require(renderer_ != nullptr, "SDL_CreateRenderer");
    vsync_ = SDL_SetRenderVSync(renderer_.get(), 1);
    resources_ = std::make_unique<Resources>(renderer_.get());
    focused_ = (SDL_GetWindowFlags(window_.get()) & SDL_WINDOW_INPUT_FOCUS) != 0;
    minimized_ = false;
    input_ = {};
    clock_.reset();
    running_ = true;
}

int Windows::run() {
    try {
        initialize();
        auto previous = SDL_GetTicksNS();
        while (running_) {
            const auto frameStart = SDL_GetTicksNS();
            double elapsed = static_cast<double>(frameStart - previous) / 1'000'000'000.0;
            previous = frameStart;
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                handleEvent(event);
                // 恢复窗口时丢弃后台时间，不进行补帧。
                if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED ||
                    event.type == SDL_EVENT_WINDOW_RESTORED) { elapsed = 0; }
            }
            if (!running_) { break; }
            if (focused_ && !minimized_) {
                clock_.advance(elapsed, [this](double dt) {
                    update(dt);
                    input_.finishStep();
                });
            } else {
                clock_.reset();
                input_.reset();
            }
            if (!minimized_) { render(); }
            // 无垂直同步或后台状态时主动限速。
            const Uint64 budget = (!focused_ || minimized_) ? 50'000'000 : 16'666'667;
            const auto spent = SDL_GetTicksNS() - frameStart;
            if ((!vsync_ || !focused_ || minimized_) && spent < budget) {
                SDL_DelayNS(budget - spent);
            }
        }
        shutdown();
        return 0;
    } catch (const std::exception& error) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", error.what());
        shutdown();
        return 1;
    }
}

void Windows::handleEvent(const SDL_Event& event) {
    input_.handleEvent(event);
    switch (event.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: running_ = false; break;
    case SDL_EVENT_WINDOW_FOCUS_LOST: focused_ = false; clock_.reset(); break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED: focused_ = true; break;
    case SDL_EVENT_WINDOW_MINIMIZED: minimized_ = true; break;
    case SDL_EVENT_WINDOW_RESTORED: minimized_ = false; break;
    default: break;
    }
}

void Windows::update(double deltaTime) {
    (void)deltaTime;
    // TODO: 在固定时间步内消费 input_，更新菜单及游戏状态。
}

void Windows::render() {
    // 使用窗口当前尺寸的默认视口；缩放窗口无需重建渲染器。
    require(SDL_SetRenderDrawColor(renderer_.get(), 24, 28, 34, 255), "Draw color");
    require(SDL_RenderClear(renderer_.get()), "Render clear");
    showMainMenu();
    require(SDL_RenderPresent(renderer_.get()), "Render present");
}

void Windows::shutdown() {
    running_ = false;
    resources_.reset();
    renderer_.reset();
    window_.reset();
    if (ttfInitialized_) { TTF_Quit(); ttfInitialized_ = false; }
    if (sdlInitialized_) { SDL_Quit(); sdlInitialized_ = false; }
    input_.reset();
    clock_.reset();
}

void Windows::showMainMenu() {
    // TODO: 实现开始菜单的布局、绘制和按钮交互。
}

} // namespace zombie
