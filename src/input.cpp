#include "zombie/input.h"

namespace zombie {
namespace {
void change(ButtonState& state, bool down) {
    if (down && !state.held) { state.pressed = true; }
    if (!down && state.held) { state.released = true; }
    state.held = down;
}
}

void Input::handleEvent(const SDL_Event& event) {
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        if (!event.key.repeat && event.key.scancode > SDL_SCANCODE_UNKNOWN &&
            event.key.scancode < SDL_SCANCODE_COUNT) {
            change(keys_[event.key.scancode], event.type == SDL_EVENT_KEY_DOWN);
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        mouseX_ = event.button.x;
        mouseY_ = event.button.y;
        if (event.button.button > 0 && event.button.button < mouseButtons_.size()) {
            change(mouseButtons_[event.button.button], event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
        }
        break;
    case SDL_EVENT_MOUSE_MOTION:
        mouseX_ = event.motion.x;
        mouseY_ = event.motion.y;
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        wheelY_ += event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -event.wheel.y : event.wheel.y;
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        reset();
        break;
    default:
        break;
    }
}

void Input::finishStep() {
    for (auto& key : keys_) { key.pressed = key.released = false; }
    for (auto& button : mouseButtons_) { button.pressed = button.released = false; }
    wheelY_ = 0;
}

void Input::reset() {
    keys_ = {};
    mouseButtons_ = {};
    wheelY_ = 0;
}

} // namespace zombie
