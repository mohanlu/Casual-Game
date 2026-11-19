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

目前没有 `src/main.cpp`。CMake 会在构建目录内生成临时探针，检查 SDL3、图片库、字体库和 EnTT 的编译链接；检查成功后可以正常生成构建文件，此时没有游戏可执行文件。

后续创建 `src/main.cpp` 后，重新运行构建脚本即可生成 `build/Debug/bin/ZombieGame.exe` 或 `build/Release/bin/ZombieGame.exe`；新增其他源文件时在 `CMakeLists.txt` 的目标中显式添加。

入口实现后可使用：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Run
```

`-Run` 会临时加入对应配置的依赖 DLL 路径，并以可执行文件目录为工作目录运行。

## VS Code

打开 `.vscode/zombie-game.code-workspace`，或直接打开 `D:\shixi\zombie-game` 文件夹，然后安装推荐的 C/C++、CMake Tools 扩展。

- `Ctrl+Shift+B`：Debug 构建。
- “运行任务”：安装依赖、仅配置、Debug 构建、Release 构建。
- CMake 预设：`windows-debug`、`windows-release`。
- `F5`：使用 UCRT64 GDB 调试，待游戏入口实现后可用。

尚无源码时，IntelliSense 使用手动配置的依赖头文件路径；创建入口并构建后生成 `compile_commands.json`，供编辑器读取实际编译选项。

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
- 游戏入口尚未创建；窗口、资源加载和运行时玩法验收随下一阶段进行。
