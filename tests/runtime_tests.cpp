#include "zombie/windows.h"
#include <SDL3_image/SDL_image.h>
#include <cmath>
#include <iostream>
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
            check(menu.handleEvent(event) == first, "Menu first action");
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
        std::cout << "Clock passed" << std::endl;
        testInput();
        std::cout << "Input passed" << std::endl;
        testResources();
        std::cout << "Resources passed" << std::endl;
        testGraphics();
        std::cout << "Graphics passed" << std::endl;
        testGameFlow();
        testMenus();
        std::cout << "Game flow and menus passed" << std::endl;
        testLifecycle();
        std::cout << "Runtime foundation checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
