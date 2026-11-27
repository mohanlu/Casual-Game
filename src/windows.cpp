#include "zombie/windows.h"
#include "zombie/viewport.h"
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
    settings_ = {};
    settingsOpen_ = false;
    controlsOpen_ = false;
    settingsStatus_.clear();
    char* preferences = SDL_GetPrefPath("ZombieGame", "ZombieGame");
    settingsPath_ = preferences ? std::filesystem::u8path(preferences) / "settings.ini" : std::filesystem::path{};
    SDL_free(preferences);
    if (!settingsPath_.empty()) { settings_.loadDisplay(settingsPath_); }
    window_.reset(SDL_CreateWindow("Zombie Game", 960, 720,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY));
    require(window_ != nullptr, "SDL_CreateWindow");
    if (settings_.fullscreen && (!SDL_SetWindowFullscreen(window_.get(), true) ||
        !SDL_SyncWindow(window_.get()) || !(SDL_GetWindowFlags(window_.get()) & SDL_WINDOW_FULLSCREEN))) {
        SDL_SetWindowFullscreen(window_.get(), false);
        settings_.fullscreen = false;
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Fullscreen unavailable: %s", SDL_GetError());
    }
    renderer_.reset(SDL_CreateRenderer(window_.get(), nullptr));
    require(renderer_ != nullptr, "SDL_CreateRenderer");
    vsync_ = SDL_SetRenderVSync(renderer_.get(), 1);
    updateViewport();
    require(SDL_SetRenderDrawBlendMode(renderer_.get(), SDL_BLENDMODE_BLEND), "UI blending");
    resources_ = std::make_unique<Resources>(renderer_.get());
    graphics_ = std::make_unique<GameGraphics>();
    graphics_->load(*resources_, renderer_.get());
    flow_ = GameFlow{};
    menu_ = std::make_unique<Menu>();
    refreshState();
    focused_ = (SDL_GetWindowFlags(window_.get()) & SDL_WINDOW_INPUT_FOCUS) != 0;
    minimized_ = false;
    input_ = {};
    clock_.reset(SDL_GetTicksNS());
    running_ = true;
}

int Windows::run() {
    try {
        initialize();
        while (running_) {
            const auto frameStart = SDL_GetTicksNS();
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                // 先结算事件发生前的运行时间，再切换暂停／恢复状态。
                const auto now = SDL_GetTicksNS();
                advanceClock(event.common.timestamp == 0 ? now : std::min(now, event.common.timestamp));
                updateViewport();
                require(SDL_ConvertEventToRenderCoordinates(renderer_.get(), &event), "Input coordinates");
                handleEvent(event);
                clock_.setRunning(running_ && focused_ && !minimized_ && flow_.state() == GameState::Playing);
                if (!running_) { break; }
            }
            if (!running_) { break; }
            advanceClock(SDL_GetTicksNS());
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
    if (flow_.state() == GameState::Playing) { input_.handleEvent(event); }
    switch (event.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: running_ = false; break;
    case SDL_EVENT_WINDOW_FOCUS_LOST: focused_ = false; input_.reset(); break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED: focused_ = true; break;
    case SDL_EVENT_WINDOW_MINIMIZED: minimized_ = true; input_.reset(); break;
    case SDL_EVENT_WINDOW_RESTORED: minimized_ = false; break;
    default: break;
    }
    if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) { menu_->handleEvent(event); }
    if (!running_ || !focused_) { return; }
    if (settingsOpen_) {
        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat && event.key.scancode == SDL_SCANCODE_ESCAPE) {
            applyAction(MenuAction::Back);
        } else { applyAction(menu_->handleEvent(event)); }
        return;
    }
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat &&
        (event.key.scancode == SDL_SCANCODE_ESCAPE || event.key.scancode == SDL_SCANCODE_SPACE || event.key.scancode == SDL_SCANCODE_P)) {
        if (flow_.state() == GameState::Playing) {
            flow_.pause();
            refreshState();
        } else if (flow_.state() == GameState::Paused) {
            flow_.resume();
            refreshState();
        }
        return;
    }
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat && flow_.state() == GameState::Playing &&
        (event.key.scancode == SDL_SCANCODE_V || event.key.scancode == SDL_SCANCODE_X)) {
        finishGame(event.key.scancode == SDL_SCANCODE_V);
        return;
    }
    applyAction(menu_->handleEvent(event));
}

void Windows::advanceClock(Uint64 now) {
    clock_.advanceTo(now, [this](double dt) {
        update(dt);
        input_.finishStep();
    });
    clock_.setRunning(running_ && focused_ && !minimized_ && flow_.state() == GameState::Playing);
}

void Windows::update(double deltaTime) {
    if (flow_.state() != GameState::Playing) { return; }
    flow_.update(deltaTime);
    // TODO: 在此接入玩家、僵尸和补给更新；胜败调用 finishGame()。
}

void Windows::updateViewport() {
    const auto size = fitViewport(renderer_.get());
    viewWidth_ = size.width; viewHeight_ = size.height;
    if (menu_) { menu_->setSize(viewWidth_, viewHeight_); }
}

void Windows::render() {
    updateViewport();
    require(SDL_SetRenderDrawColor(renderer_.get(), 24, 28, 34, 255), "Draw color");
    require(SDL_RenderClear(renderer_.get()), "Render clear");
    if (flow_.state() == GameState::Playing) {
        for (int y = 0; y < viewHeight_; y += 64) {
            for (int x = 0; x < viewWidth_; x += 64) {
                graphics_->sprite(SpriteId::ground).draw(renderer_.get(),
                    {static_cast<float>(x), static_cast<float>(y), 64, 64});
            }
        }
    }
    menu_->draw(renderer_.get());
    require(SDL_RenderPresent(renderer_.get()), "Render present");
}

void Windows::shutdown() {
    running_ = false;
    menu_.reset();
    graphics_.reset();
    resources_.reset();
    renderer_.reset();
    window_.reset();
    if (ttfInitialized_) { TTF_Quit(); ttfInitialized_ = false; }
    if (sdlInitialized_) { SDL_Quit(); sdlInitialized_ = false; }
    input_.reset();
    clock_.reset(0);
}

void Windows::refreshState() {
    input_.reset();
    menu_->setSize(viewWidth_, viewHeight_);
    menu_->rebuild(flow_.state(), *resources_, renderer_.get(), settings_, settingsOpen_, settingsStatus_.c_str(), controlsOpen_);
}

void Windows::applyAction(MenuAction action) {
    bool changed = false;
    switch (action) {
    case MenuAction::Start:
        changed = flow_.startGame(settings_.zombieCount);
        if (changed) { clock_.startNewGame(); }
        break;
    case MenuAction::Resume: changed = flow_.resume(); break;
    case MenuAction::MainMenu: changed = flow_.returnToMainMenu(); break;
    case MenuAction::Settings:
        settingsOpen_ = true;
        controlsOpen_ = false;
        settingsStatus_.clear();
        changed = true;
        break;
    case MenuAction::Controls:
        if (settingsOpen_) { controlsOpen_ = true; changed = true; }
        break;
    case MenuAction::Back:
        if (controlsOpen_) { controlsOpen_ = false; }
        else { settingsOpen_ = false; }
        changed = true;
        break;
    case MenuAction::Zombies50:
    case MenuAction::Zombies100:
    case MenuAction::Zombies200:
        if (settingsOpen_ && !controlsOpen_ && flow_.state() == GameState::MainMenu) {
            settings_.zombieCount = action == MenuAction::Zombies50 ? 50 : action == MenuAction::Zombies100 ? 100 : 200;
            changed = true;
        }
        break;
    case MenuAction::Windowed: changeDisplay(false); changed = true; break;
    case MenuAction::Fullscreen: changeDisplay(true); changed = true; break;
    case MenuAction::Quit: running_ = false; break;
    default: break;
    }
    if (changed) { refreshState(); }
}

void Windows::changeDisplay(bool fullscreen) {
    if (!SDL_SetWindowFullscreen(window_.get(), fullscreen) || !SDL_SyncWindow(window_.get()) ||
        ((SDL_GetWindowFlags(window_.get()) & SDL_WINDOW_FULLSCREEN) != 0) != fullscreen) {
        settings_.fullscreen = (SDL_GetWindowFlags(window_.get()) & SDL_WINDOW_FULLSCREEN) != 0;
        settingsStatus_ = "显示模式切换失败，请重试";
        return;
    }
    settings_.fullscreen = fullscreen;
    settingsStatus_ = settings_.saveDisplay(settingsPath_) ? "显示设置已保存" : "保存失败，下次启动可能无法恢复";
}

bool Windows::finishGame(bool won) {
    if (!running_ || !flow_.finishGame(won)) { return false; }
    refreshState();
    return true;
}

} // namespace zombie
