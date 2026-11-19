# Casual-Game

C++ 2D 俯视角僵尸生存游戏，项目目录名为 `zombie-game`。

- [开发工作清单](worklist.md)
- [Windows 环境与构建说明](docs/SETUP_WINDOWS.md)

当前阶段：搭建开发环境，游戏入口与玩法将在后续实现。

技术栈：C++17、SDL3、SDL3_image、SDL3_ttf、EnTT、CMake、Ninja、vcpkg。

## 目录安排

| 目录／文件 | 用途 |
|---|---|
| `config/cmake/` | 构建预设、依赖导入与链接检查 |
| `config/vcpkg/` | 依赖清单和平台编译配置 |
| `scripts/` | 环境安装、构建和工具链脚本 |
| `docs/` | 环境与使用说明 |
| `.vscode/` | 编辑器设置和工作区入口 |
| `CMakeLists.txt`、`CMakePresets.json` | CMake 在根目录查找的构建入口 |
| `build/`、`vcpkg_installed/` | 本机生成的构建产物和依赖，不纳入 Git |

后续 C++ 头文件放入 `include/`，实现文件放入 `src/`。
