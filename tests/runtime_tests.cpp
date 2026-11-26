#include "zombie/windows.h"
#include <SDL3_image/SDL_image.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <thread>

namespace {
void check(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

void testClock() {
    for (int fps : {30, 60, 144}) {
        zombie::FixedStep clock;
        int count = 0;
        for (int frame = 0; frame < fps * 2; ++frame) {
            clock.advance(1.0 / fps, [&](double dt) {
                check(std::abs(dt - 1.0 / 60) < 1e-12, "Fixed dt");
                ++count;
            });
        }
        check(count == 120, "Render rate changed logic rate");
    }
    zombie::FixedStep clock;
    int count = 0;
    clock.advance(5, [&](double) { ++count; });
    check(count == 15, "Long stall must cap catch-up");
    clock.advance(0.01, [&](double) { ++count; });
    clock.reset();
    clock.advance(0.01, [&](double) { ++count; });
    check(count == 15, "Reset must discard accumulated time");
}

void testPauseClock() {
    zombie::GameClock clock;
    zombie::GameFlow flow;
    check(flow.startGame(), "Start clock test session");
    int steps = 0;
    double cooldown = 0.1;
    auto update = [&](double dt) {
        ++steps;
        flow.update(dt);
        cooldown -= dt;
    };
    clock.reset(0);
    clock.setRunning(true);
    clock.advanceTo(10'000'000, update); // 暂停前累计 10 ms，不足一步。
    check(steps == 0, "Substep must remain pending");
    flow.pause();
    clock.setRunning(false);
    clock.advanceTo(5'000'000'000, update);
    clock.advanceTo(10'000'000'000, update); // 包括停留在设置页的时间。
    check(steps == 0 && flow.playTime() == 0 && cooldown == 0.1, "Pause must freeze game time and cooldown");
    flow.resume();
    clock.setRunning(true);
    clock.advanceTo(10'006'000'000, update);
    check(steps == 0, "Resume must not catch up paused wall time");
    clock.advanceTo(10'006'666'667, update);
    check(steps == 1 && std::abs(flow.playTime() - zombie::FixedStep::interval) < 1e-12,
        "Resume must use the saved 10 ms remainder");
    check(std::abs(cooldown - (0.1 - zombie::FixedStep::interval)) < 1e-12, "Cooldown continues from saved progress");

    // 对比连续运行与反复暂停：活跃时间相同，逻辑步数及最终余量必须相同。
    zombie::GameClock continuous, interrupted;
    continuous.reset(0);
    interrupted.reset(0);
    continuous.setRunning(true);
    interrupted.setRunning(true);
    std::uint64_t activeTime = 0, wallTime = 0;
    int continuousSteps = 0, interruptedSteps = 0;
    auto countContinuous = [&](double) { ++continuousSteps; };
    auto countInterrupted = [&](double) { ++interruptedSteps; };
    for (int i = 0; i < 100; ++i) {
        activeTime += 5'000'000;
        wallTime += 5'000'000;
        continuous.advanceTo(activeTime, countContinuous);
        interrupted.advanceTo(wallTime, countInterrupted);
        interrupted.setRunning(false);
        wallTime += 2'000'000'000;
        interrupted.advanceTo(wallTime, countInterrupted);
        interrupted.setRunning(true);
        check(continuousSteps == interruptedSteps, "Repeated pause loses active time");
    }
    check(interruptedSteps == 30, "500 ms active time must produce 30 steps");
    continuous.advanceTo(activeTime + 16'666'667, countContinuous);
    interrupted.advanceTo(wallTime + 16'666'667, countInterrupted);
    check(continuousSteps == 31 && interruptedSteps == 31, "Pause must preserve residual phase");

    // 新一局才清空余量；迟到的事件不能倒退时间。
    clock.reset(0);
    clock.setRunning(true);
    steps = 0;
    clock.advanceTo(10'000'000, update);
    clock.startNewGame();
    clock.advanceTo(16'000'000, update);
    check(steps == 0, "New game must clear old remainder");
    clock.advanceTo(5'000'000, update);
    clock.advanceTo(20'000'000, update);
    check(steps == 0, "Stale timestamps must not double count time");
    clock.advanceTo(26'666'667, update);
    check(steps == 1, "New game clock must advance normally");
}

void testInput() {
    zombie::Input input;
    SDL_Event event{};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.scancode = SDL_SCANCODE_W;
    input.handleEvent(event);
    check(input.key(SDL_SCANCODE_W).pressed && input.key(SDL_SCANCODE_W).held, "Key press");
    input.finishStep();
    event.key.repeat = true;
    input.handleEvent(event);
    check(!input.key(SDL_SCANCODE_W).pressed && input.key(SDL_SCANCODE_W).held, "Key repeat");
    event.key.repeat = false;
    event.type = SDL_EVENT_KEY_UP;
    input.handleEvent(event);
    check(input.key(SDL_SCANCODE_W).released && !input.key(SDL_SCANCODE_W).held, "Key release");
    input.finishStep();
    event.type = SDL_EVENT_KEY_DOWN;
    input.handleEvent(event);
    event.type = SDL_EVENT_KEY_UP;
    input.handleEvent(event);
    check(input.key(SDL_SCANCODE_W).pressed && input.key(SDL_SCANCODE_W).released, "Short tap lost");
    event = {};
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = 25;
    event.button.y = 40;
    input.handleEvent(event);
    check(input.mouseButton(SDL_BUTTON_LEFT).held && input.mouseX() == 25 && input.mouseY() == 40,
        "Mouse input");
    event.type = SDL_EVENT_WINDOW_FOCUS_LOST;
    input.handleEvent(event);
    check(!input.mouseButton(SDL_BUTTON_LEFT).held && !input.key(SDL_SCANCODE_W).pressed,
        "Focus loss must clear input");
}

void testResources() {
    check(SDL_Init(SDL_INIT_VIDEO), SDL_GetError());
    check(TTF_Init(), SDL_GetError());
    {
        std::unique_ptr<SDL_Window, zombie::SdlDeleter> window(SDL_CreateWindow("test", 64, 64, 0));
        check(window != nullptr, SDL_GetError());
        std::unique_ptr<SDL_Renderer, zombie::SdlDeleter> renderer(SDL_CreateRenderer(window.get(), nullptr));
        check(renderer != nullptr, SDL_GetError());
        zombie::Resources resources(renderer.get());
        const auto expectedRoot = std::filesystem::u8path(SDL_GetBasePath()) / "assets";
        check(resources.root().is_absolute() && resources.root() == expectedRoot,
            "Resources must resolve relative to the executable");
        check(resources.texture("images/atlas.png") != nullptr, "Bundled atlas");
        check(resources.font("images/ui.ttf", 18) != nullptr, "Bundled font");
        for (const char* notice : {"NOTICE.md", "licenses/wenquanyi/GPL-2.0.txt",
                 "licenses/wenquanyi/README.upstream", "licenses/wenquanyi/AUTHORS",
                 "licenses/wenquanyi/GFDL-1.2.txt", "licenses/wenquanyi/upstream-source.zip"}) {
            check(std::filesystem::is_regular_file(resources.root() / "images" / notice),
                "Bundled asset notices or font source missing");
        }
        std::filesystem::create_directories(resources.root());
        const auto fixture = resources.root() / "runtime-test.bmp";
        SDL_Surface* surface = SDL_CreateSurface(2, 2, SDL_PIXELFORMAT_RGBA32);
        check(surface != nullptr, SDL_GetError());
        const bool saved = SDL_SaveBMP(surface, fixture.u8string().c_str());
        SDL_DestroySurface(surface);
        check(saved, "Save texture fixture");
        auto* texture = resources.texture("runtime-test.bmp");
        check(texture == resources.texture("runtime-test.bmp"), "Texture cache");
        resources.clear();
        check(resources.texture("runtime-test.bmp") != nullptr, "Reload after clear");
        bool failed = false;
        try { resources.texture("missing-test-texture.png"); } catch (const std::runtime_error&) { failed = true; }
        check(failed, "Missing texture must report failure");
        failed = false;
        try { resources.font("missing-test-font.ttf", 18); } catch (const std::runtime_error&) { failed = true; }
        check(failed, "Missing font must report failure");
        std::filesystem::remove(fixture);
    }
    TTF_Quit();
    SDL_Quit();
}

void testLifecycle() {
    zombie::Windows window;
    for (Uint32 closeEvent : {SDL_EVENT_QUIT, SDL_EVENT_WINDOW_CLOSE_REQUESTED}) {
        // 从另一个线程投递关闭事件，主线程实际完成初始化、事件循环和渲染。
        std::thread stop([closeEvent] {
            SDL_Delay(200);
            SDL_Event event{};
            event.type = closeEvent;
            SDL_PushEvent(&event);
        });
        const int result = window.run();
        stop.join();
        check(result == 0, "Window lifecycle failed");
        check(SDL_WasInit(0) == 0 && TTF_WasInit() == 0, "Subsystem leak");
    }
    SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "nonexistent-test-renderer", SDL_HINT_OVERRIDE);
    check(window.run() == 1, "Initialization failure exit code");
    check(SDL_WasInit(0) == 0 && TTF_WasInit() == 0, "Partial initialization leak");
    SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
}

void testPauseSettingsFlow() {
    for (bool escapeBack : {false, true}) {
        zombie::Windows window;
        std::thread events([escapeBack] {
            SDL_Delay(200);
            auto sendKey = [](SDL_Scancode key, bool repeat = false) {
                SDL_Event e{};
                e.type = SDL_EVENT_KEY_DOWN;
                e.key.scancode = key;
                e.key.repeat = repeat;
                SDL_PushEvent(&e);
            };
            auto click = [](float x, float y) {
                SDL_Event e{};
                e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
                e.button.button = SDL_BUTTON_LEFT;
                e.button.x = x; e.button.y = y;
                SDL_PushEvent(&e);
                e.type = SDL_EVENT_MOUSE_BUTTON_UP;
                SDL_PushEvent(&e);
            };
            SDL_Event e{};
            e.type = SDL_EVENT_WINDOW_FOCUS_GAINED;
            SDL_PushEvent(&e);
            click(480, 380); // 开始游戏
            sendKey(SDL_SCANCODE_P);
            sendKey(SDL_SCANCODE_P, true); // 长按不能恢复
            click(480, 425); // 暂停 -> 设置
            sendKey(SDL_SCANCODE_SPACE); // 设置内不能恢复游戏
            sendKey(SDL_SCANCODE_P);
            click(600, 385); // 已锁定的僵尸选项
            if (escapeBack) { sendKey(SDL_SCANCODE_ESCAPE); }
            else { click(480, 570); }
            click(480, 370); // 返回暂停菜单后继续
            sendKey(SDL_SCANCODE_ESCAPE); // 再次暂停
            e.type = SDL_EVENT_QUIT;
            SDL_PushEvent(&e);
        });
        const int result = window.run();
        events.join();
        check(result == 0 && window.state() == zombie::GameState::Paused,
            "Settings must return to pause menu and preserve pause hotkeys");
    }
}

void testSettings() {
    const auto path = std::filesystem::current_path() / "test-display-settings.ini";
    std::filesystem::remove(path);
    zombie::Settings settings;
    settings.loadDisplay(path);
    check(!settings.fullscreen && settings.zombieCount == 50, "Default settings");
    settings.fullscreen = true;
    check(settings.saveDisplay(path), "Save fullscreen");
    zombie::Settings reopened;
    reopened.loadDisplay(path);
    check(reopened.fullscreen, "Restore fullscreen on reopen");
    reopened.fullscreen = false;
    check(reopened.saveDisplay(path), "Save windowed");
    settings.loadDisplay(path);
    check(!settings.fullscreen, "Restore windowed on reopen");
    { std::ofstream file(path); file << "invalid"; }
    settings.fullscreen = true;
    settings.loadDisplay(path);
    check(!settings.fullscreen, "Invalid settings fallback");
    check(!settings.saveDisplay(path / "missing"), "Save failure reported");
    std::filesystem::remove(path);
    for (int target : {50, 100, 200}) {
        zombie::GameFlow flow;
        check(!flow.startGame(75), "Reject invalid zombie count");
        check(flow.startGame(target) && flow.zombieTarget() == target, "Session uses selected count");
    }
}

void testGameFlow() {
    zombie::GameFlow flow;
    using S = zombie::GameState;
    check(flow.state() == S::MainMenu && !flow.resume() && !flow.finishGame(true), "Initial state guards");
    check(flow.startGame() && flow.state() == S::Playing, "Start game");
    flow.update(1);
    check(flow.pause() && !flow.finishGame(false), "Pause transition");
    flow.update(10);
    check(flow.playTime() == 1, "Pause must freeze world time");
    check(flow.resume() && flow.finishGame(true) && flow.state() == S::Victory, "Victory transition");
    flow.update(10);
    check(!flow.resume() && !flow.pause() && flow.playTime() == 1, "Victory must stay frozen");
    check(flow.returnToMainMenu() && flow.startGame() && flow.playTime() == 0, "Fresh session");
    check(flow.finishGame(false) && flow.state() == S::Defeat && !flow.resume(), "Defeat transition");
    check(flow.returnToMainMenu(), "Return after defeat");
}

void testMenus() {
    check(SDL_Init(SDL_INIT_VIDEO) && TTF_Init(), SDL_GetError());
    {
        std::unique_ptr<SDL_Window, zombie::SdlDeleter> window(SDL_CreateWindow("menus", 960, 720, SDL_WINDOW_RESIZABLE));
        std::unique_ptr<SDL_Renderer, zombie::SdlDeleter> renderer(SDL_CreateRenderer(window.get(), nullptr));
        check(renderer != nullptr, SDL_GetError());
        SDL_SetRenderLogicalPresentation(renderer.get(), 960, 720, SDL_LOGICAL_PRESENTATION_LETTERBOX);
        SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);
        zombie::Resources resources(renderer.get());
        zombie::GameGraphics graphics;
        graphics.load(resources, renderer.get());
        zombie::Menu menu;
        using S = zombie::GameState;
        using A = zombie::MenuAction;
        const std::array<S, 5> states{S::MainMenu, S::Playing, S::Paused, S::Victory, S::Defeat};
        const std::array<const char*, 5> names{"main-menu", "playing", "paused", "victory", "defeat"};
        for (size_t index = 0; index < states.size(); ++index) {
            menu.rebuild(states[index], resources, renderer.get());
            SDL_SetRenderDrawColor(renderer.get(), 24, 28, 34, 255);
            SDL_RenderClear(renderer.get());
            for (int y = 0; y < 720; y += 64) {
                for (int x = 0; x < 960; x += 64) {
                    graphics.sprite(zombie::SpriteId::ground).draw(renderer.get(),
                        {static_cast<float>(x), static_cast<float>(y), 64, 64});
                }
            }
            menu.draw(renderer.get());
            if (const char* output = SDL_getenv("ZOMBIE_UI_SNAPSHOTS")) {
                std::filesystem::create_directories(output);
                std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> snapshot(
                    SDL_RenderReadPixels(renderer.get(), nullptr), SDL_DestroySurface);
                check(snapshot != nullptr, SDL_GetError());
                check(IMG_SavePNG(snapshot.get(), (std::filesystem::u8path(output) /
                    (std::string(names[index]) + ".png")).u8string().c_str()), SDL_GetError());
            }
            SDL_Event event{};
            event.type = SDL_EVENT_KEY_DOWN;
            event.key.scancode = SDL_SCANCODE_RETURN;
            const A first = states[index] == S::MainMenu ? A::Start : states[index] == S::Playing ? A::None :
                states[index] == S::Paused ? A::Resume : A::MainMenu;
            check(menu.handleEvent(event) == A::None, "Keyboard confirmation removed");
            for (auto key : {SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN}) {
                event.key.scancode = key;
                check(menu.handleEvent(event) == A::None, "Keyboard navigation removed");
            }
            event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
            event.button.button = SDL_BUTTON_LEFT;
            event.button.x = 480; event.button.y = 380;
            menu.handleEvent(event);
            event.type = SDL_EVENT_MOUSE_BUTTON_UP;
            check(menu.handleEvent(event) == first, "Menu first mouse action");
            event = {};
            event.type = SDL_EVENT_KEY_DOWN;
            event.key.scancode = SDL_SCANCODE_RETURN;
            event.key.repeat = true;
            check(menu.handleEvent(event) == A::None, "Repeated confirmation");
        }
        menu.rebuild(S::MainMenu, resources, renderer.get());
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        event.button.button = SDL_BUTTON_LEFT;
        event.button.x = 480; event.button.y = 380;
        check(menu.handleEvent(event) == A::None, "Mouse down should not activate");
        event.type = SDL_EVENT_WINDOW_FOCUS_LOST;
        menu.handleEvent(event);
        event.type = SDL_EVENT_MOUSE_BUTTON_UP;
        check(menu.handleEvent(event) == A::None, "Focus loss cancels click");
        event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        menu.handleEvent(event);
        event.type = SDL_EVENT_MOUSE_BUTTON_UP;
        check(menu.handleEvent(event) == A::Start, "Mouse click activation");
        auto click = [&](float x, float y) {
            SDL_Event e{};
            e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
            e.button.button = SDL_BUTTON_LEFT;
            e.button.x = x; e.button.y = y;
            menu.handleEvent(e);
            e.type = SDL_EVENT_MOUSE_BUTTON_UP;
            return menu.handleEvent(e);
        };
        check(click(480, 450) == A::Settings, "Main menu settings entry");
        check(click(480, 520) == A::Quit, "Main menu quit");
        zombie::Settings settings;
        settings.zombieCount = 100;
        menu.rebuild(S::MainMenu, resources, renderer.get(), settings, true);
        SDL_RenderClear(renderer.get());
        menu.draw(renderer.get());
        if (const char* output = SDL_getenv("ZOMBIE_UI_SNAPSHOTS")) {
            std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> snapshot(
                SDL_RenderReadPixels(renderer.get(), nullptr), SDL_DestroySurface);
            check(snapshot && IMG_SavePNG(snapshot.get(), (std::filesystem::u8path(output) / "settings.png").u8string().c_str()), SDL_GetError());
        }
        check(click(350, 385) == A::Zombies50 && click(475, 385) == A::Zombies100 &&
            click(600, 385) == A::Zombies200, "Zombie choices");
        check(click(380, 485) == A::Windowed && click(570, 485) == A::Fullscreen, "Display choices");
        check(click(480, 570) == A::Back && click(250, 650) == A::None, "Settings return and outside click");
        menu.rebuild(S::Paused, resources, renderer.get(), settings);
        check(click(480, 425) == A::Settings, "Pause menu settings entry");
        check(click(480, 485) == A::MainMenu && click(480, 545) == A::Quit, "Pause menu remaining buttons");
        menu.rebuild(S::Paused, resources, renderer.get(), settings, true);
        check(click(350, 385) == A::None && click(475, 385) == A::None &&
            click(600, 385) == A::None, "Paused zombie choices must be disabled");
        check(click(380, 485) == A::Windowed && click(570, 485) == A::Fullscreen,
            "Paused display choices remain enabled");
        check(click(480, 570) == A::Back, "Paused settings return");
        SDL_RenderClear(renderer.get());
        menu.draw(renderer.get());
        if (const char* output = SDL_getenv("ZOMBIE_UI_SNAPSHOTS")) {
            std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> snapshot(
                SDL_RenderReadPixels(renderer.get(), nullptr), SDL_DestroySurface);
            check(snapshot && IMG_SavePNG(snapshot.get(), (std::filesystem::u8path(output) / "paused-settings.png").u8string().c_str()), SDL_GetError());
        }
        if (std::string(SDL_GetCurrentVideoDriver()) != "dummy") {
            check(SDL_SetWindowFullscreen(window.get(), true) && SDL_SyncWindow(window.get()), "Enter fullscreen");
            check((SDL_GetWindowFlags(window.get()) & SDL_WINDOW_FULLSCREEN) != 0, "Fullscreen flag");
            float x = 0, y = 0;
            check(SDL_RenderCoordinatesToWindow(renderer.get(), 480, 485, &x, &y), "Fullscreen coordinate mapping");
            SDL_Event mapped{};
            mapped.type = SDL_EVENT_MOUSE_BUTTON_UP;
            mapped.button.windowID = SDL_GetWindowID(window.get());
            mapped.button.x = x; mapped.button.y = y;
            check(SDL_ConvertEventToRenderCoordinates(renderer.get(), &mapped), "Fullscreen input mapping");
            check(std::abs(mapped.button.x - 480) < 1 && std::abs(mapped.button.y - 485) < 1, "Fullscreen click position");
            check(SDL_SetWindowFullscreen(window.get(), false) && SDL_SyncWindow(window.get()), "Leave fullscreen");
            check((SDL_GetWindowFlags(window.get()) & SDL_WINDOW_FULLSCREEN) == 0, "Windowed flag");
        }
        SDL_SetWindowSize(window.get(), 1280, 720);
        SDL_PumpEvents();
        event.button.windowID = SDL_GetWindowID(window.get());
        event.button.x = 640; event.button.y = 380;
        check(SDL_ConvertEventToRenderCoordinates(renderer.get(), &event), SDL_GetError());
        check(std::abs(event.button.x - 480) < 1 && std::abs(event.button.y - 380) < 1,
            "Resized letterbox mouse coordinates");
    }
    TTF_Quit();
    SDL_Quit();
}

void testGraphics() {
    check(SDL_Init(SDL_INIT_VIDEO) && TTF_Init(), SDL_GetError());
    {
        std::unique_ptr<SDL_Window, zombie::SdlDeleter> window(SDL_CreateWindow("graphics", 64, 64, 0));
        std::unique_ptr<SDL_Renderer, zombie::SdlDeleter> renderer(SDL_CreateRenderer(window.get(), nullptr));
        check(renderer != nullptr, SDL_GetError());
        const auto root = std::filesystem::u8path(SDL_GetBasePath()) / "graphics-fixture";
        std::filesystem::create_directories(root / "images");
        zombie::Resources resources(renderer.get(), root);
        auto saveImage = [&](int width, int height, const char* name) {
            std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(
                SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
            check(surface != nullptr, SDL_GetError());
            SDL_FillSurfaceRect(surface.get(), nullptr, SDL_MapSurfaceRGB(surface.get(), 0, 255, 0));
            SDL_Rect left{0, 0, 16, 32}, right{16, 0, 16, 32};
            SDL_FillSurfaceRect(surface.get(), &left, SDL_MapSurfaceRGB(surface.get(), 255, 0, 0));
            SDL_FillSurfaceRect(surface.get(), &right, SDL_MapSurfaceRGB(surface.get(), 0, 0, 255));
            check(IMG_SavePNG(surface.get(), (root / "images" / name).u8string().c_str()), SDL_GetError());
        };
        auto pixel = [&](int x, int y, Uint8 red, Uint8 green, Uint8 blue) {
            SDL_Rect area{x, y, 1, 1};
            std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(
                SDL_RenderReadPixels(renderer.get(), &area), SDL_DestroySurface);
            check(surface != nullptr, SDL_GetError());
            Uint8 r, g, b, a;
            check(SDL_ReadSurfacePixel(surface.get(), 0, 0, &r, &g, &b, &a), SDL_GetError());
            check(r == red && g == green && b == blue, "Unexpected rendered pixel");
        };
        saveImage(32, 32, "player.png");
        zombie::GameGraphics graphics;
        graphics.load(resources, renderer.get());
        check(graphics.sprite(zombie::SpriteId::player).texture != nullptr, "Separate PNG loading");
        check(graphics.sprite(zombie::SpriteId::health).texture == nullptr, "Missing asset handling");
        saveImage(160, 64, "atlas.png");
        graphics.load(resources, renderer.get());
        const SDL_FRect destination{0, 0, 32, 32};
        graphics.sprite(zombie::actorSprite(zombie::ActorType::player, zombie::VisualState::normal))
            .draw(renderer.get(), destination, 90);
        pixel(16, 8, 255, 0, 0);
        pixel(16, 24, 0, 0, 255);
        graphics.sprite(zombie::actorSprite(zombie::ActorType::player, zombie::VisualState::hurt))
            .draw(renderer.get(), destination);
        pixel(16, 16, 0, 255, 0);
        check(graphics.sprite(zombie::actorSprite(zombie::ActorType::zombie, zombie::VisualState::dead))
            .source.y == 32, "Death state selects second row");
        zombie::Resources bundledResources(renderer.get());
        zombie::TextLabel label;
        auto* font = bundledResources.font("images/ui.ttf", 18);
        check(font == bundledResources.font("images/ui.ttf", 18), "Font cache");
        label.set(renderer.get(), font, "HP 100");
        check(label.width() > 0 && label.height() > 0, "Text texture dimensions");
        label.draw(renderer.get(), 0, 32);
        label.set(renderer.get(), font, "");
        check(label.width() == 0 && label.height() == 0, "Clear text");
        std::filesystem::remove(root / "images/player.png");
        std::filesystem::remove(root / "images/atlas.png");
        std::filesystem::remove(root / "images");
        std::filesystem::remove(root);
    }
    TTF_Quit();
    SDL_Quit();
}
}

int main() {
    try {
        testClock();
        testPauseClock();
        std::cout << "Clock passed" << std::endl;
        testInput();
        std::cout << "Input passed" << std::endl;
        testResources();
        std::cout << "Resources passed" << std::endl;
        testGraphics();
        std::cout << "Graphics passed" << std::endl;
        testSettings();
        testGameFlow();
        testMenus();
        std::cout << "Game flow and menus passed" << std::endl;
        testPauseSettingsFlow();
        testLifecycle();
        std::cout << "Runtime foundation checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
