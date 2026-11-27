#pragma once

#include "zombie/game_flow.h"
#include "zombie/graphics.h"
#include "zombie/settings.h"
#include <vector>

namespace zombie {
enum class MenuAction { None, Start, Resume, MainMenu, Quit, Settings, Controls, Back,
    Zombies50, Zombies100, Zombies200, Windowed, Fullscreen };

class Menu {
public:
    void rebuild(GameState state, Resources& resources, SDL_Renderer* renderer,
        const Settings& settings = {}, bool settingsOpen = false, const char* status = "", bool controlsOpen = false);
    MenuAction handleEvent(const SDL_Event& event);
    void draw(SDL_Renderer* renderer) const;
    void setSize(float width, float height);

private:
    struct Button {
        SDL_FRect rectangle;
        MenuAction action;
        TextLabel label;
        bool option = false;
        bool checked = false;
        bool enabled = true;
        SDL_Texture* icon = nullptr; // 由 Resources 持有。
    };
    int hit(float x, float y) const;
    GameState state_ = GameState::MainMenu;
    TextLabel eyebrow_, title_, subtitle_, footer_;
    TextLabel zombieLabel_, displayLabel_, status_;
    bool settingsOpen_ = false;
    bool controlsOpen_ = false;
    std::vector<TextLabel> controlKeys_, controlDescriptions_;
    std::vector<Button> buttons_;
    int selected_ = -1;
    int pressed_ = -1;
    float width_ = 960, height_ = 720;
};
} // namespace zombie
