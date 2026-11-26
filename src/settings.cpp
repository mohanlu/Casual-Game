#include "zombie/settings.h"
#include <fstream>
#include <string>

namespace zombie {
void Settings::loadDisplay(const std::filesystem::path& path) {
    std::ifstream file(path);
    std::string value;
    std::getline(file, value);
    fullscreen = value == "fullscreen=1";
}

bool Settings::saveDisplay(const std::filesystem::path& path) const {
    std::ofstream file(path, std::ios::trunc);
    file << "fullscreen=" << (fullscreen ? 1 : 0) << '\n';
    file.close();
    return !file.fail();
}
} // namespace zombie
