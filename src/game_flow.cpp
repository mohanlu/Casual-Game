#include "zombie/game_flow.h"

namespace zombie {
void GameFlow::enter(GameState state) { state_ = state; ++revision_; }

bool GameFlow::startGame() {
    if (state_ != GameState::MainMenu) { return false; }
    playTime_ = 0;
    enter(GameState::Playing);
    return true;
}
bool GameFlow::pause() {
    if (state_ != GameState::Playing) { return false; }
    enter(GameState::Paused);
    return true;
}
bool GameFlow::resume() {
    if (state_ != GameState::Paused) { return false; }
    enter(GameState::Playing);
    return true;
}
bool GameFlow::finishGame(bool won) {
    if (state_ != GameState::Playing) { return false; }
    enter(won ? GameState::Victory : GameState::Defeat);
    return true;
}
bool GameFlow::returnToMainMenu() {
    if (state_ == GameState::MainMenu || state_ == GameState::Playing) { return false; }
    enter(GameState::MainMenu);
    return true;
}
void GameFlow::update(double deltaTime) {
    if (state_ == GameState::Playing && deltaTime > 0) { playTime_ += deltaTime; }
}
} // namespace zombie
