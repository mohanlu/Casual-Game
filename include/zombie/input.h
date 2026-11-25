#pragma once

#include <SDL3/SDL.h>
#include <array>

namespace zombie {

struct ButtonState {
    bool held = false;
    bool pressed = false;
    bool released = false;
};

class Input {
public:
    void handleEvent(const SDL_Event& event);
    void finishStep();
    void reset();
    const ButtonState& key(SDL_Scancode code) const { return keys_.at(code); }
    const ButtonState& mouseButton(Uint8 button) const { return mouseButtons_.at(button); }
    float mouseX() const { return mouseX_; }
    float mouseY() const { return mouseY_; }
    float wheelY() const { return wheelY_; }

private:
    std::array<ButtonState, SDL_SCANCODE_COUNT> keys_{};
    std::array<ButtonState, 6> mouseButtons_{};
    float mouseX_ = 0;
    float mouseY_ = 0;
    float wheelY_ = 0;
};

} // namespace zombie
