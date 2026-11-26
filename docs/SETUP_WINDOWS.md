# Windows 开发环境

本项目沿用旧项目的 MSYS2 UCRT64 GCC、CMake、Ninja 和 vcpkg 工具，依赖单独安装在新项目的 `vcpkg_installed` 中。

## 工具与路径

| 工具 | 本机位置 |
|---|---|
| GCC／G++／GDB | `C:\msys64\ucrt64\bin` |
| vcpkg | `D:\vcpkg` |
| CMake 4.4.4 | `D:\vcpkg\downloads\tools\cmake-4.4.4-windows\cmake-4.4.4-windows-x86_64\bin\cmake.exe` |
| Ninja 1.13.2 | `D:\vcpkg\downloads\tools\ninja-1.13.2-windows\ninja.exe` |

构建脚本仅在运行期间设置 PATH，优先使用 UCRT64 编译器，结束后恢复原值。

## 安装项目依赖

在本项目目录打开 PowerShell：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\setup.ps1
```

依赖由 `config/vcpkg/vcpkg.json` 的 baseline 固定；EnTT 额外固定为 3.12.0。目标及宿主 triplet 均为 `x64-mingw-dynamic`，同时安装 Debug 和 Release 库。

| 直接依赖 | 版本 | 用途 |
|---|---|---|
| SDL3 | 3.4.20 | 窗口、输入、渲染 |
| SDL3_image | 3.4.8，启用 PNG | 图集加载 |
| SDL3_ttf | 3.2.2，port revision 1 | 字体绘制 |
| EnTT | 3.12.0 | 实体组件管理 |

依赖安装与游戏构建分开执行，日常构建不自动下载依赖。

`config/vcpkg/vcpkg-configuration.json` 指向项目内的 triplet。由于本机 GCC 16.2 在编译 SDL3 的 `SDL_blit_auto.c` 时触发 `ch_vect` 内部错误，triplet 仅为 SDL3 的 Release C 编译关闭自动向量化；其余依赖及游戏代码使用正常优化配置。安装依赖的并行数设为 4。

## 配置与构建

```powershell
# 检查工具链及依赖的头文件、C++17 编译与链接
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -ConfigureOnly

# Debug 配置与构建
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1

# Release 配置与构建
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Configuration Release
```

`src/main.cpp` 仅调用 `Windows::run()`。程序创建 960×720 可调整大小的窗口并显示开始菜单。开始后进入场景占位页，Esc／空格暂停和继续；结算画面仅允许返回菜单或退出。界面等比例缩放，鼠标坐标转换到逻辑视口。

逻辑按 60 Hz 固定步长更新，渲染优先使用垂直同步，无垂直同步时限速至约 60 FPS。失焦或最小化时停止逻辑更新并清空按键状态，恢复时丢弃后台经过的时间；单帧最多追赶 0.25 秒。

`Resources` 从可执行文件旁的 `assets/` 加载并缓存纹理与字体，加载失败报告文件路径及 SDL 错误。返回的指针由缓存持有，调用 `clear()` 或销毁缓存后失效。释放顺序为资源缓存、渲染器、窗口、TTF、SDL。具体图集及字体素材随后接入。

运行构建脚本生成 `build/Debug/bin/ZombieGame.exe` 或 `build/Release/bin/ZombieGame.exe`；新增其他源文件时在 `CMakeLists.txt` 的目标中显式添加。

构建并运行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Run
```

`-Run` 会临时加入对应配置的依赖 DLL 路径，并以可执行文件目录为工作目录运行。

运行基础测试：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Test
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Configuration Release -Test
```

测试由 CTest 使用 SDL 的无显示视频驱动运行，覆盖不同渲染频率下的固定更新时间、短按和重复按键、失焦清理、纹理缓存、资源加载失败、重复启动关闭和初始化失败清理。

## VS Code

打开 `.vscode/zombie-game.code-workspace`，或直接打开 `D:\shixi\zombie-game` 文件夹，然后安装推荐的 C/C++、CMake Tools 扩展。

- `Ctrl+Shift+B`：Debug 构建。
- “运行任务”：安装依赖、仅配置、Debug 构建、Release 构建。
- CMake 预设：`windows-debug`、`windows-release`。
- `F5`：使用 UCRT64 GDB 调试，可在 `main` 或 `Windows::run()` 设置断点。

构建后生成 `compile_commands.json`，供 IntelliSense 读取实际编译选项。

## 文件职责与路径调整

| 文件 | 职责 |
|---|---|
| `config/vcpkg/vcpkg.json` | 依赖及版本锁定 |
| `config/vcpkg/vcpkg-configuration.json`、`config/vcpkg/triplets/` | 本项目的 MinGW 配置与 SDL3 编译兼容选项 |
| `CMakeLists.txt` | 根目录构建入口、C++ 标准和游戏目标 |
| `CMakePresets.json` | 根目录预设入口，引用实际配置 |
| `config/cmake/Dependencies.cmake` | 依赖导入和编译链接检查 |
| `config/cmake/windows-presets.json` | Debug／Release 的输出目录和本机默认工具链 |
| `scripts/toolchain.ps1` | 检查脚本使用的工具位置 |
| `scripts/setup.ps1` | 安装依赖 |
| `scripts/build.ps1` | 配置、构建及后续运行 |
| `.vscode/` | 补全、构建任务和 GDB 调试 |
| `.editorconfig`、`.gitattributes` | 编码、缩进及换行 |
| `.gitignore` | 忽略编译产物、依赖安装目录和本机个人配置 |

换机器时，脚本支持 `-VcpkgRoot` 和 `-CompilerRoot` 参数；CMake／Ninja 的版本路径在 `scripts/toolchain.ps1` 中调整。直接使用 CMake Tools 时，同步调整 `config/cmake/windows-presets.json` 和 `.vscode` 中的工具路径，或通过根目录下未纳入 Git 的 `CMakeUserPresets.json` 添加本机预设。

Release 发布包的 DLL／资源收集与打包在 D08 完成。

## 已验证（2026-11-19）

- 项目依赖全部安装完成，包含 Debug／Release 两种配置；SDL3 使用上述兼容选项成功编译。
- `scripts/build.ps1` 的 Debug、Release 配置及构建命令通过；C++17 探针同时引用 SDL3、SDL3_image、SDL3_ttf、EnTT，编译链接成功。
- PowerShell 脚本及 JSON 配置通过语法检查，构建和依赖目录符合 Git 忽略规则。
- 当时已建立游戏入口及 TODO 接口骨架。

## 运行基础验证（2026-11-25）

- Debug、Release 构建与 CTest 运行基础测试通过。
- 使用本机真实视频驱动完成短时窗口创建、渲染和正常关闭；初始化失败返回非零退出码并清理子系统。
- 从项目外工作目录运行测试仍能加载可执行文件旁的测试资源。
- 菜单绘制、图集与字体素材接入、具体游戏逻辑留在后续阶段实现。

## 菜单与状态验证（2026-11-26）

- 接入用户字体，检查 MainMenu、Playing、Paused、Victory、Defeat 五种状态的实际渲染截图，中文及按钮布局正常。
- Debug／Release 测试通过：状态转换限制、暂停／结算时间冻结、新局计时重置、键盘确认、鼠标点击与失焦取消、缩放后坐标转换。
- `GameFlow` 定义状态规则；`Menu` 定义界面；`Windows::finishGame(bool won)` 是后续战斗逻辑的胜败入口。
- 截图可通过环境变量 `ZOMBIE_UI_SNAPSHOTS` 指定输出目录后执行 `scripts/build.ps1 -Test` 导出。

## 资源分发验证（2026-11-26）

- `Resources` 默认从可执行文件所在目录的 `assets/` 加载；初始化失败时报告具体原因。
- CTest 在独立的 `build/<配置>/test-working-directory/` 工作目录执行，测试真实图集、随包字体及说明文件加载；测试不再读取 Windows 系统字体。
- Debug／Release 构建及测试通过。构建复制完整 `src/images/`，包括资源说明、字体许可证、嵌入例外、作者名单及固定版本的上游源文件归档。
- 用户的 `ui.ttf` 与上游 WenQuanYi Bitmap Song 16px 文件逐字节一致；来源、版本及 SHA-256 记录在 [随包资源说明](../src/images/NOTICE.md)。
