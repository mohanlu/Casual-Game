#pragma once

#include "zombie/fixed_step.h"
#include <cstdint>

namespace zombie {

// 墙上时间只在运行区间内进入固定步长调度器。
// 暂停仍更新时间基准，但保留调度器内尚不足一步的余量。
class GameClock {
public:
    void reset(std::uint64_t now) {
        steps_.reset();
        previous_ = now;
        running_ = false;
    }
    void startNewGame() { steps_.reset(); }
    void setRunning(bool running) { running_ = running; }

    template<class Update>
    void advanceTo(std::uint64_t now, Update update) {
        // 迟到的事件不能让时间倒退或重复计时。
        now = std::max(now, previous_);
        const double elapsed = static_cast<double>(now - previous_) / 1'000'000'000.0;
        previous_ = now;
        if (running_) { steps_.advance(elapsed, update); }
    }

private:
    FixedStep steps_;
    std::uint64_t previous_ = 0;
    bool running_ = false;
};

} // namespace zombie
