# OFFSET Launcher

OFFSET 是一个使用原生 x86 汇编编写的 Minecraft Java 版启动器，面向 Windows 10/11 64 位系统。

[下载 Windows 启动器（OFFSET.exe）](https://github.com/Yesaz0erq/OFFSET-Launcher/raw/refs/heads/main/downloads/OFFSET.exe)

## 功能

- Minecraft 正式版、快照和多版本管理。可在「版本管理 → 已安装」移除版本。恢复时，将游戏目录下 `.offset-version-trash/` 中对应的文件夹按原版本名移回 `versions/`。
- Fabric、Forge、NeoForge、Quilt 加载器
- Microsoft 正版登录与离线启动
- Modrinth 模组搜索、下载和管理；支持常用中文名、简称和繁体搜索
- Modrinth、CurseForge、Prism / MultiMC 和 MCBBS 整合包导入与导出
- 每版本独立设置、Java 自动选择和 JVM 参数
- 通过文件夹/文件选择器设置游戏目录与 Java，并可自动检测本机已安装的 Java
- 服务器列表、崩溃日志与主题设置
- 个性化：默认使用带点阵背景与制图网格的蓝图主题，可自选配色，也可切换为 Minecraft 官网风格
- 深色模式：浅色 / 深色 / 跟随系统，输入框、列表与右键菜单同步切换

## 整合包互通

在「整合包 → 导入」选择本地文件，可预览包名、Minecraft 版本、加载器、下载项和包内文件数。实例目录名支持中文。重新选择同一个包和目录，可以继续上次中断的导入。已有完整实例时会建议带编号的新目录。在「整合包 → 导出」选择已安装的版本、目标格式及内容。存档与服务器列表可单独勾选。

| 导入与导出格式 | 文件内容 |
| --- | --- |
| Modrinth `.mrpack` | 读取 `modrinth.index.json`，校验下载文件，依次应用 `overrides/` 和 `client-overrides/`。导出当前客户端实例。 |
| CurseForge ZIP | 读取 `manifest.json` 和指定的覆盖目录，按项目 ID 和文件 ID 获取必需文件。配置 API Key 时使用 CurseForge 官方元数据，未配置时使用 MCIM 公共元数据镜像。文件按 SHA-1 校验。若作者限制第三方下载，可将官网下载的同名文件放在整合包旁重试，也会检查同名的本机 Prism 实例。 |
| Prism Launcher / MultiMC ZIP | 读取 `mmc-pack.json`、`instance.cfg` 和 `.minecraft/` 或 `minecraft/`。支持空实例及标准加载器组件。 |
| MCBBS ZIP（HMCL / PCL） | 读取 `mcbbs.packmeta` 或含 `addons` 的 `manifest.json`，校验内置文件的 SHA-1，并解析 CurseForge 文件引用。导出生成 MCBBS 与 CurseForge 双清单。 |

从此版本起，通过模组页下载或从整合包安装的文件会在实例中保留来源。再次导出到同一平台时，路径和内容匹配的文件写入下载清单。修改过的文件和缺少来源记录的旧文件随包保存。

支持根目录清单或单层文件夹包裹的 ZIP。单包上限为 1 GiB、16,384 个 ZIP 条目，单个内置文件上限为 1 GiB，解压后总量须小于 2 GiB。标准加载器包括 Fabric、Forge、NeoForge 和 Quilt。MCBBS 支持本地包的安装快照，纯 `fileApi` 远程补全、自定义运行库以及 Prism 自定义组件补丁超出当前支持范围。

## 构建

需要 Windows PowerShell、FASM 1.73.32 和 64 位 Java（运行 Minecraft 时使用）。

```powershell
.\build.ps1
.\package.ps1
```

构建产物位于 `dist/`。启动器源码位于 `src/`，FASM 运行时和头文件位于 `tools/`。

## 许可

项目代码采用仓库中的 `LICENSE`。第三方组件和许可见 `THIRD_PARTY.md`。
