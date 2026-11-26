# Demo 图片与字体清单

## 当前素材（2026-11-26）

已使用内置 image_gen 生成下面列出的 10 张图片，并转换为 32×32 PNG，保存至 `src/images/`。同时提供按既定顺序拼接的 `atlas.png`（160×64），程序优先加载图集。

玩家和僵尸各有正常、受伤、死亡三种状态；角色与物品保留透明通道，地面与墙体为不透明贴图。用户提供的 `ui.ttf` 已接入中文菜单。

- [放大预览](assets-preview.png)
- [生成提示词记录](imagegen-prompts.json)（内置工具生成；尺寸转换与图集拼装为确定性处理）

若后续修改独立 PNG，需要同步更新图集，或移走 `atlas.png` 后重新构建以切换到独立图片加载模式。

把以下文件放到 `src/images/`。推荐先收集独立 PNG；代码也支持按本文布局合成的图集，两种方式任选一种。

## 方案一：独立图片（推荐）

所有图片均为 **32×32 像素、PNG 格式**。角色和物品使用透明背景，地面与墙体铺满格子。

| 文件名 | 内容 | 画面要求 |
|---|---|---|
| `player.png` | 玩家正常状态 | 俯视角，角色中心对准格子中心，初始朝右 |
| `player_hurt.png` | 玩家受伤状态 | 与正常图位置、朝向一致，伤势或颜色变化清楚 |
| `player_dead.png` | 玩家死亡状态 | 倒地形态，仍以格子中心为定位点 |
| `zombie.png` | 基础僵尸正常状态 | 俯视角，初始朝右，与玩家风格及比例一致 |
| `zombie_hurt.png` | 僵尸受伤状态 | 与正常图位置、朝向一致 |
| `zombie_dead.png` | 僵尸死亡状态 | 倒地形态；战斗阶段再决定死亡图的停留时长 |
| `ammo.png` | 弹药补给 | 弹药盒或子弹图标，透明背景 |
| `health.png` | 回血物品 | 医疗包或恢复药品，与未来解毒药丸区别开 |
| `ground.png` | 可通行地面 | 四边可连续平铺，俯视纹理，不画透视边框 |
| `wall.png` | 不可通行墙体／障碍 | 俯视方块，与地面明显区分，可重复排列 |

默认不需要行走动画、多个朝向的图片：角色朝向由代码旋转。上述图片可以是从素材包中裁出的单帧；请保持透明度和人物中心一致。

## 字体（两种图片方案都需要）

| 文件名 | 要求 |
|---|---|
| `ui.ttf` | TrueType 字体，包含中文、英文字母、数字及常用标点；支持随游戏分发 |

字体放在 `src/images/ui.ttf`。目前使用文泉驿点阵宋体 16px 的 TTF 转换版，文件与上游固定提交逐字节一致，只改文件名。字号由各界面指定。

## 资源定位与随包分发

`Resources` 使用 `SDL_GetBasePath()` 定位可执行文件目录，默认资源根为其下的 `assets/`；不使用当前工作目录，也不回退到源码目录或系统字体。菜单加载 `images/ui.ttf`，图集加载 `images/atlas.png`。

构建会递归复制 `src/images/` 中的素材、来源说明、许可证和字体上游源文件，输出布局如下（Debug／Release 相同）：

```text
bin/
  ZombieGame.exe
  assets/images/
    atlas.png、各独立 PNG、ui.ttf
    NOTICE.md
    licenses/wenquanyi/
      GPL-2.0.txt
      README.upstream
      AUTHORS
      GFDL-1.2.txt
      CONVERSION.upstream.md
      upstream-source.zip
```

发布时保留完整 `assets/` 目录。图片来源、字体版权及 GPL v2 字体嵌入例外见 [随包资源说明](../src/images/NOTICE.md)。字体的源文件归档和转换说明随包提供，不需要收件人联网获取。替换字体时同时更新授权说明及源文件，不能直接沿用原字体的许可证。

本项完成资源准备；完整 Release 包的 DLL 收集和打包仍属于 D08。

## 方案二：一张图集

使用 `src/images/atlas.png`，**160×64 像素，5 列×2 行，每格 32×32，格子之间无空隙、无外边距**。排列固定如下：

| 行／列（从 0 开始） | 0 | 1 | 2 | 3 | 4 |
|---|---|---|---|---|---|
| 0 | 玩家正常 | 玩家受伤 | 玩家死亡 | 僵尸正常 | 僵尸受伤 |
| 1 | 僵尸死亡 | 弹药 | 回血物品 | 地面 | 墙体 |

`atlas.png` 存在时优先使用整张图集，不再读取独立 PNG。选择独立图片方案时不要放 `atlas.png`。改变方案后重新构建，会同步移除构建输出目录中已删除的 PNG／TTF。

## 放入后如何查看

在项目根目录运行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Run
```

构建会把 `src/images/` 同步到程序旁的 `assets/images/`；运行时不依赖当前工作目录。目前启动进入正式菜单布局，已替换原素材预览页。素材总览见本文的放大预览链接；`GameGraphics::drawPreview()` 接口仍可用于开发检查。

缺少的图片会在控制台输出提示；菜单需要 `ui.ttf`。尺寸错误、文件损坏或缺少菜单字体会报告错误并退出。运行中替换素材后，需要退出、重新构建并运行。

## 已完成的代码接口

- `GameGraphics::load()`：加载整图集或独立图片，并加载预览字体。
- `Sprite::source`：默认 32×32 的源选区；`draw()` 支持指定显示矩形、绕中心顺时针旋转，角度单位为度。
- `actorSprite(ActorType, VisualState)`：玩家／僵尸的正常、受伤、死亡状态对应图片。
- `TextLabel::set()`／`draw()`：UTF-8 文字生成纹理并绘制；文字、字体、颜色不变时复用纹理。
- `Resources`：缓存原始纹理与字体。预览与文字纹理先释放，再释放缓存和渲染器。

状态选图的用法：

```cpp
const auto id = zombie::actorSprite(zombie::ActorType::player, zombie::VisualState::hurt);
graphics.sprite(id).draw(renderer, {x, y, 32, 32}, angleDegrees);
```

实际伤害事件、死亡停留时长、墙体碰撞与回血效果在对应玩法阶段接入。当前素材已接入并通过图像与中文菜单渲染检查。
