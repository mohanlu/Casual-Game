#pragma once

#include "zombie/game_flow.h"
#include "zombie/graphics.h"
#include <vector>

namespace zombie {
enum class MenuAction { None, Start, Resume, MainMenu, Quit };

class Menu {
public:
    void rebuild(GameState state, Resources& resources, SDL_Renderer* renderer);
    MenuAction handleEvent(const SDL_Event& event);
    void draw(SDL_Renderer* renderer) const;

private:
    struct Button {
        SDL_FRect rectangle;
        MenuAction action;
        TextLabel label;
    };
    int hit(float x, float y) const;
    GameState state_ = GameState::MainMenu;
    TextLabel eyebrow_, title_, subtitle_, footer_;
    std::vector<Button> buttons_;
    int selected_ = 0;
    int pressed_ = -1;
};
} // namespace zombie
