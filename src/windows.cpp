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
    require(SDL_SetRenderLogicalPresentation(renderer_.get(), 960, 720, SDL_LOGICAL_PRESENTATION_LETTERBOX), "Logical viewport");
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
            const auto revision = flow_.revision();
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                require(SDL_ConvertEventToRenderCoordinates(renderer_.get(), &event), "Input coordinates");
                handleEvent(event);
                // 恢复窗口时丢弃后台时间，不进行补帧。
                if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED ||
                    event.type == SDL_EVENT_WINDOW_RESTORED) { elapsed = 0; }
            }
            if (revision != flow_.revision()) { elapsed = 0; }
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
    if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) { menu_->handleEvent(event); }
    if (!running_ || !focused_) { return; }
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat &&
        (event.key.scancode == SDL_SCANCODE_ESCAPE || event.key.scancode == SDL_SCANCODE_SPACE)) {
        if (flow_.state() == GameState::Playing) {
            flow_.pause();
            refreshState();
        } else if (flow_.state() == GameState::Paused) {
            flow_.resume();
            refreshState();
        }
        return;
    }
    applyAction(menu_->handleEvent(event));
}

void Windows::update(double deltaTime) {
    if (flow_.state() != GameState::Playing) { return; }
    flow_.update(deltaTime);
    // TODO: 在此接入玩家、僵尸和补给更新；胜败调用 finishGame()。
}

void Windows::render() {
    require(SDL_SetRenderDrawColor(renderer_.get(), 24, 28, 34, 255), "Draw color");
    require(SDL_RenderClear(renderer_.get()), "Render clear");
    for (int y = 0; y < 720; y += 64) {
        for (int x = 0; x < 960; x += 64) {
            graphics_->sprite(SpriteId::ground).draw(renderer_.get(),
                {static_cast<float>(x), static_cast<float>(y), 64, 64});
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
    clock_.reset();
}

void Windows::refreshState() {
    clock_.reset();
    input_.reset();
    menu_->rebuild(flow_.state(), *resources_, renderer_.get());
}

void Windows::applyAction(MenuAction action) {
    bool changed = false;
    switch (action) {
    case MenuAction::Start: changed = flow_.startGame(); break;
    case MenuAction::Resume: changed = flow_.resume(); break;
    case MenuAction::MainMenu: changed = flow_.returnToMainMenu(); break;
    case MenuAction::Quit: running_ = false; break;
    default: break;
    }
    if (changed) { refreshState(); }
}

bool Windows::finishGame(bool won) {
    if (!running_ || !flow_.finishGame(won)) { return false; }
    refreshState();
    return true;
}

} // namespace zombie
