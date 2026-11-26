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

- `MainMenu`：开始游戏、设置、退出游戏。
- 设置页：僵尸数量通过单选框选择 50／100／200（默认 50，新一局采用当前选值）；显示设置通过单选框选择小窗／全屏，立即生效并自动保存，下次启动恢复。点击“返回”或按 Esc 返回主菜单。
- `Playing`：场景占位；Esc、空格或 P 进入暂停。
- `Paused`：继续游戏、设置、返回主菜单、退出；Esc、空格或 P 也可继续。暂停中的设置页锁定僵尸数量，显示模式仍可调整并保存；点击返回或按 Esc 回到暂停菜单，游戏保持暂停。
- `Victory / Defeat`：共用结算布局，提供返回主菜单、退出；不能恢复已结束的一局。
- 菜单仅用鼠标点击操作，已移除方向键、W/S 和 Enter 的菜单操作。窗口缩放时按 960×720 等比例布局并换算点击坐标。

显示配置通过 SDL 用户配置目录保存到 `settings.ini`（Windows 通常为 `%APPDATA%\ZombieGame\ZombieGame\settings.ini`），不依赖启动目录。首次启动或配置无效时默认 960×720 小窗；保存失败会在设置页提示。僵尸数量本次运行有效，尚未接入实际刷怪逻辑。

后续战斗逻辑在主线程调用 `Windows::finishGame(true)`／`finishGame(false)` 进入胜利／失败。当前没有自动胜败条件。只有窗口处于前台且未最小化的 `Playing` 推进世界更新时间。暂停、暂停设置页、失焦和最小化均保留不足一个逻辑步的时间余量，恢复后继续累计，不补算停顿期间的时间；只有开始新一局才清空上一局的计时进度。状态切换按输入事件时间戳结算边界，刷新菜单不重置时钟。运行中长时间卡顿仍保留原有的单次最多 0.25 秒追帧限制。

```powershell
# 构建并运行
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Run

# 构建并执行运行时测试
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Test
```
