#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace zombie {

struct SdlDeleter {
    void operator()(SDL_Window* value) const { SDL_DestroyWindow(value); }
    void operator()(SDL_Renderer* value) const { SDL_DestroyRenderer(value); }
    void operator()(SDL_Texture* value) const { SDL_DestroyTexture(value); }
    void operator()(TTF_Font* value) const { TTF_CloseFont(value); }
};

// 借用 renderer；本对象必须先于 renderer 和 TTF 子系统销毁。
class Resources {
public:
    // 默认根目录为可执行文件旁的 assets，独立于进程的当前工作目录。
    // 显式 root 用于测试或工具指定资源目录。
    explicit Resources(SDL_Renderer* renderer, std::filesystem::path root = {});
    SDL_Texture* texture(const std::string& relativePath);
    TTF_Font* font(const std::string& relativePath, float pointSize);
    const std::filesystem::path& root() const { return root_; }
    void clear();

private:
    SDL_Renderer* renderer_;
    std::filesystem::path root_;
    std::map<std::string, std::unique_ptr<SDL_Texture, SdlDeleter>> textures_;
    std::map<std::pair<std::string, float>, std::unique_ptr<TTF_Font, SdlDeleter>> fonts_;
};

} // namespace zombie
