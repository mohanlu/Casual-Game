#pragma once

namespace zombie {

enum class GameState { MainMenu, Playing, Paused, Victory, Defeat };

class GameFlow {
public:
    GameState state() const { return state_; }
    unsigned revision() const { return revision_; }
    double playTime() const { return playTime_; }
    bool startGame(int zombieCount = 50);
    int zombieTarget() const { return zombieTarget_; }
    bool pause();
    bool resume();
    bool finishGame(bool won);
    bool returnToMainMenu();
    void update(double deltaTime);

private:
    void enter(GameState state);
    GameState state_ = GameState::MainMenu;
    unsigned revision_ = 0;
    double playTime_ = 0;
    int zombieTarget_ = 50;
};

} // namespace zombie
