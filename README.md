# OFFSET Launcher

OFFSET 是一个使用原生 x86 汇编编写的 Minecraft Java 版启动器，面向 Windows 10/11 64 位系统。

## 功能

- Minecraft 正式版、快照和多版本管理
- Fabric、Forge、NeoForge、Quilt 加载器
- Microsoft 正版登录与离线启动
- Modrinth 模组搜索、下载和管理；支持常用中文名、简称和繁体搜索
- Modrinth、CurseForge、Prism 整合包导入与导出
- 每版本独立设置、Java 自动选择和 JVM 参数
- 通过文件夹/文件选择器设置游戏目录与 Java，并可自动检测本机已安装的 Java
- 服务器列表、崩溃日志与主题设置
- 个性化：窗口颜色、背景纹理（点阵/网格/方块/纯色）与一键配色方案
- 深色模式：浅色 / 深色 / 跟随系统，输入框、列表与右键菜单同步切换

## 构建

需要 Windows PowerShell、FASM 1.73.32 和 64 位 Java（运行 Minecraft 时使用）。

```powershell
.\build.ps1
.\package.ps1
```

构建产物位于 `dist/`。启动器源码位于 `src/`，FASM 运行时和头文件位于 `tools/`。

## 许可

项目代码采用仓库中的 `LICENSE`。第三方组件和许可见 `THIRD_PARTY.md`。
