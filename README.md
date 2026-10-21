# OFFSET

独立开发的 Minecraft Java 版 Windows 启动器，使用原创 x86 汇编编写，界面采用浅色卡片与蓝图风格。

An independently developed Windows launcher for Minecraft: Java Edition, written in original x86 assembly. This repository contains only the inputs required to build the launcher.

## 构建 / Build

环境：Windows 10/11、Windows PowerShell 5.1 或 PowerShell 7。

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1
```

输出为 `dist/OFFSET.exe`。构建会先从 `src/strings.json` 生成界面与状态文字，再调用随源码保留的 FASM 1.73.32。构建不需要联网，不依赖 .NET SDK、Node.js、Python 或 Java。

The build generates the language tables and assembles `dist/OFFSET.exe`. The required FASM compiler and 24 header/macro files are included for an offline build. Their license is in `tools/LICENSE.TXT`.

## 使用 / Use

运行 `dist/OFFSET.exe`。运行 Minecraft 另需安装对应 Java；游戏、加载器、模组及整合包按用户操作单独下载。

支持版本与模组管理、整合包导入和导出、独立版本设置、主题色及启动日志。微软登录需使用开发者自己的 Application (client) ID，并取得相应 Minecraft Services API 权限；当前登录会话仅保存在进程内存。

Minecraft and Java are not included. Microsoft sign-in requires the launcher's own app registration and Minecraft Services access approval.

## 目录 / Contents

- `src/`：汇编源码、语言源数据、程序清单与实际使用的图标。
- `tools/`：FASM 编译器、构建所需头文件、语言表生成脚本及工具许可证。
- `build.ps1`：唯一构建入口。

此仓库不包含发布成品、游戏文件、账户配置、密钥、测试和缓存、备份、截图或开发过程文档。`src/strings.inc` 与 `src/strings_status.inc` 是自动生成的文件。

原创代码使用 MIT 许可证；构建工具与第三方服务见 `THIRD_PARTY.md`。OFFSET 与 Mojang、Microsoft 及其他启动器项目无隶属关系，也不是官方 Minecraft 产品。
