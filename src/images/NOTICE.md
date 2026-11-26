# 随包资源来源与分发说明

核对日期：2026-11-26。本文件适用于同目录内的图片和字体，随游戏资源一起保留。

## 图片

`player.png`、`player_hurt.png`、`player_dead.png`、`zombie.png`、`zombie_hurt.png`、`zombie_dead.png`、`ammo.png`、`health.png`、`ground.png`、`wall.png` 为本项目按用户要求通过 OpenAI 内置 image_gen 生成的素材，随后缩放为 32×32；`atlas.png` 由这些图片拼装而成。生成记录在项目的 `docs/imagegen-prompts.json`。

这些图片作为本项目素材随游戏分发，不属于文泉驿字体的 GPL 授权范围。OpenAI 使用条款中的 Content 条款说明：在用户与 OpenAI 之间、适用法律允许的范围内，输出权利归用户；输出可能并非唯一。本说明记录素材来源，不另行为图片指定开源许可证。

来源：[OpenAI 使用条款](https://openai.com/policies/terms-of-use/#content)。

## 字体

`ui.ttf` 是 **WenQuanYi Bitmap Song 16px（文泉驿点阵宋体 16px）v1.0.0-RC1**，由 AmusementClub（娱乐部）将文泉驿点阵宋体转换为 TTF。本项目只改了文件名，文件内容与上游完全一致。

- 上游：[AmusementClub/WenQuanYi-Bitmap-Song-TTF](https://github.com/AmusementClub/WenQuanYi-Bitmap-Song-TTF)
- 固定提交：`7724da71817090ba92e76d9d40e3dd43afef25de`
- 上游文件：`WenQuanYi Bitmap Song 16px.ttf`
- `ui.ttf` SHA-256：`09ebd60aa1900d8d09b61dfc786cbe51454c38b26f0374ead786d7ab96de51be`
- 版权：© 2004–2010 The WenQuanYi Project Board of Trustees and Qianqian Fang；上游另列 Firefly 及文泉驿贡献者，完整名单见 `licenses/wenquanyi/AUTHORS`。
- 字体许可证：GNU GPL v2，附字体嵌入例外。完整 GPL 见 `licenses/wenquanyi/GPL-2.0.txt`；版权及例外原文见 `licenses/wenquanyi/README.upstream`。

随包提供 `licenses/wenquanyi/upstream-source.zip`，为上述固定提交的完整上游归档，包含原始 PCF 字体、TTF、作者和转换说明。归档 SHA-256：`0b93a5d14280c18804366114c2ebd46417ef8c2591425042494af7c1f993d1b9`。转换步骤另见 `licenses/wenquanyi/CONVERSION.upstream.md`，16px 对应 `Original Fonts/wenquanyi_12pt.pcf`。

分发字体时一并保留上述版权、许可证、例外和源文件归档；对字体的再分发及修改遵循其 GPL 条款。上游 README 文档采用 GNU FDL 1.2 或后续版本，附带的 `licenses/wenquanyi/GFDL-1.2.txt` 为 1.2 原文。

GNU FDL 原文来源：[SPDX 官方许可证文本库](https://github.com/spdx/license-list-data/blob/main/text/GFDL-1.2-only.txt)；[GNU 官方说明](https://www.gnu.org/licenses/old-licenses/fdl-1.2.html)。

## 包内布局

程序从可执行文件所在目录加载 `assets/images/`。移动或发布游戏时保留整个目录，包括本文件和 `licenses/`，不要只复制 PNG 和 TTF。资源无需安装到 Windows 字体目录，也无需从网络下载才能运行。

替换素材时同步更新来源、许可证及对应源文件；不要沿用与新素材不符的说明。
