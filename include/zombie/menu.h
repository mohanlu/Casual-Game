#pragma once

#include "zombie/game_flow.h"
#include "zombie/graphics.h"
#include "zombie/settings.h"
#include <vector>

namespace zombie {
enum class MenuAction { None, Start, Resume, MainMenu, Quit, Settings, Back,
    Zombies50, Zombies100, Zombies200, Windowed, Fullscreen };

class Menu {
public:
    void rebuild(GameState state, Resources& resources, SDL_Renderer* renderer,
        const Settings& settings = {}, bool settingsOpen = false, const char* status = "");
    MenuAction handleEvent(const SDL_Event& event);
    void draw(SDL_Renderer* renderer) const;

private:
    struct Button {
        SDL_FRect rectangle;
        MenuAction action;
        TextLabel label;
        bool option = false;
        bool checked = false;
        bool enabled = true;
    };
    int hit(float x, float y) const;
    GameState state_ = GameState::MainMenu;
    TextLabel eyebrow_, title_, subtitle_, footer_;
    TextLabel zombieLabel_, displayLabel_, status_;
    bool settingsOpen_ = false;
    std::vector<Button> buttons_;
    int selected_ = -1;
    int pressed_ = -1;
};
} // namespace zombie
