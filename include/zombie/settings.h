#pragma once

#include <filesystem>

namespace zombie {
struct Settings {
    int zombieCount = 50;
    bool fullscreen = false;
    void loadDisplay(const std::filesystem::path& path);
    bool saveDisplay(const std::filesystem::path& path) const;
};
} // namespace zombie
