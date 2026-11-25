#pragma once

#include <algorithm>

namespace zombie {

class FixedStep {
public:
    static constexpr double interval = 1.0 / 60.0;

    template<class Update>
    void advance(double elapsed, Update update) {
        // 限制卡顿后的追帧量，避免长时间阻塞事件处理。
        accumulator_ += std::clamp(elapsed, 0.0, 0.25);
        while (accumulator_ + 1e-12 >= interval) {
            update(interval);
            accumulator_ = std::max(0.0, accumulator_ - interval);
        }
    }

    void reset() { accumulator_ = 0.0; }

private:
    double accumulator_ = 0.0;
};

} // namespace zombie
