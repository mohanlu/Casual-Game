# Casual-Game

C++ 2D 俯视角僵尸生存游戏，项目目录名为 `zombie-game`。

- [开发工作清单](worklist.md)
- [Windows 环境与构建说明](docs/SETUP_WINDOWS.md)
- [图片与字体清单](docs/ASSETS.md)
- [随包资源来源与授权说明](src/images/NOTICE.md)

当前阶段：窗口运行基础、素材绘制及五种界面状态已实现。启动进入开始菜单，选择开始后进入场景占位页；具体玩家、僵尸及战斗玩法将在后续实现。

技术栈：C++17、SDL3、SDL3_image、SDL3_ttf、EnTT、CMake、Ninja、vcpkg。

资源从可执行文件旁的 `assets/` 加载，与启动时的工作目录无关。构建自动复制图片、字体及授权／源文件说明；移动游戏时保留完整的 `assets/` 目录。

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

C++ 头文件放入 `include/`，实现文件放入 `src/`。`src/main.cpp` 仅调用启动入口；`Windows` 管理窗口及循环，`Input` 保存输入状态，`FixedStep` 驱动固定更新，`Resources` 管理纹理和字体缓存。`GameFlow` 管理状态，`Menu` 负责界面绘制及按钮操作。

## 界面与操作

- `MainMenu`：开始游戏、退出游戏。
- `Playing`：场景占位；Esc 或空格进入暂停。
- `Paused`：继续游戏、返回主菜单、退出；Esc 或空格也可继续。
- `Victory / Defeat`：共用结算布局，提供返回主菜单、退出；不能恢复已结束的一局。
- 菜单支持鼠标点击、方向键或 W/S 选择、Enter 确认。窗口缩放时按 960×720 等比例布局并换算点击坐标。

后续战斗逻辑在主线程调用 `Windows::finishGame(true)`／`finishGame(false)` 进入胜利／失败。当前没有自动胜败条件。只有 `Playing` 推进世界更新时间，开始新一局会重置该计时。

```powershell
# 构建并运行
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Run

# 构建并执行运行时测试
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Test
```
