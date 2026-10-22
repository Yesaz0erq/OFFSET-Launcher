# OFFSET Launcher 0.9

使用原创 x86 汇编编写的 Minecraft Java 版启动器。0.4 采用受 Apple 启发的蓝图风格界面：浅色留白、柔和阴影的圆角卡片、点阵蓝图底纹、图纸式标注，以及弹簧动画与淡入转场；支持 **中文 / English / 日本語** 三种界面语言。图形使用系统自带的 GDI+ 抗锯齿绘制，文字保留 GDI ClearType，无需打包图片、字体或网页运行时。

启动器单个 EXE 约 **320.5 KiB**（含三种语言文案与多尺寸图标），以实际构建输出为准。这个体积不包含 Java、Minecraft 客户端、依赖库和模组文件。便携包为 `dist/OFFSET-0.9.0-windows.zip`。

## 运行

- Windows 10 / 11 **64 位**；启动器自身为 x86，可调用 64 位 Java。
- 下载游戏需要网络；运行已下载完整的游戏可选择离线身份。
- Minecraft 所需 Java 需单独安装。**Minecraft 1.21.1 使用 64 位 Java 21**。
- 部分版本的本地依赖解压使用 Windows 自带的 `tar.exe`。

将 `dist/OFFSET.exe` 放在当前用户可写入的文件夹中，双击运行。便携运行只需要这个 EXE；默认在 EXE 旁创建 `.minecraft` 游戏目录和 `offset.ini` 设置文件。

首次使用：

1. 在“设置”中确认游戏目录、Java 路径和内存。程序优先查找 `JAVA_HOME/bin/javaw.exe`，然后查找 `PATH` 中的 `javaw.exe`；也可以填写 `java.exe` 或 `javaw.exe` 的完整路径。
2. 打开“版本管理 → 下载”，选择正式版、快照或全部，可按版本号搜索。选择版本后点击“下载”；也可用 Ctrl / Shift 多选原版，点击“加入任务”，再到“任务”开始批量下载。
3. 需要加载器时，选中一个游戏版本，再选择 Fabric、Forge、NeoForge 或 Quilt 及其兼容版本。程序先安装基础游戏，再安装加载器及其依赖。
4. 在“已安装”中选中目标，点击“设为当前”，然后返回“开始游戏”选择身份并启动。

原版下载采用 [PCL-CE 使用的 Mojang / BMCLAPI 地址规则](https://github.com/PCL-community/PCL-CE/blob/1a7653127d578efdb2a9388999a3fdc67d9e49d5/Plain%20Craft%20Launcher%202/Modules/Minecraft/ModDownload.cs#L2136-L2244)，使用镜像优先策略：版本配置、客户端及资源索引走 `bmclapi2.bangbang93.com`；支持库依次尝试 `/maven`、`/libraries`；资源文件走 `/assets`。下载或校验失败后尝试 Mojang 官方地址，已有文件通过 SHA-1 后直接复用。版本清单保留包含 SHA-1 的 `version_manifest_v2.json` 格式，镜像缺少目标版本时也会查询官方清单。

最大内存以 MiB 为单位，下限为 `512`，上限在启动时自动读取本机物理内存总量，界面会显示当前范围。保存设置和实际启动均检查这个上限；在内存较小的机器上读取到过大的旧设置时，会自动收敛到本机上限。

## 界面、动效与语言

- **语言**：侧栏底部和“设置”页都有 中文 / EN / 日本語 切换，立即生效（任务进行中暂不可切换），并以 `Language=zh|en|ja` 写入 `offset.ini`。首次运行跟随 Windows 显示语言（中文 → 中文，日文 → 日本語，其余 → English）。界面文字、列表项、输入提示和后台任务状态均会切换；字体分别使用 Microsoft YaHei UI、Segoe UI 与 Yu Gothic UI。
- **动效**：导航选中块、分段控件滑块和账户单选在切换时以弹簧曲线滑动；切换页面或标签时旧页面淡出、新页面上浮淡入；按钮有悬停高光与按下反馈；首页蓝图卡片会以扫描方式“绘制”标志；状态文字淡入；任务进行时显示旋转指示与进度线。
- **启动动画**：启动时整窗蓝图底板上先“绘制”出 OFFSET 标志与字标，随后底板收缩为首页蓝图卡片，标志平滑移动并缩放到卡片右侧的大标志位置，全程为矢量绘制。点击或按任意键可跳过；在“个性化”中关闭动效，或在 `offset.ini` 写入 `StartupAnimation=0`，即不播放。
- **侧栏收起**：点击侧栏“WORKSPACE”行右侧的面板按钮（或按 `Ctrl+B`）可收起为仅显示图标的窄栏，内容区随之居中；收起时悬停图标显示名称，底部语言按钮点击即在 中文 / EN / 日本語 间轮换。状态以 `SidebarCollapsed=0|1` 保存在 `offset.ini`。导航不再显示 1–8 编号，`Ctrl+1`–`Ctrl+9` 快捷键仍然可用。
- **无边框窗口**：去掉系统标题栏，保留系统阴影、圆角与任务栏最小化行为。右上角为自绘的最小化与关闭按钮（关闭悬停变红）；窗口顶部空白区域可直接拖动，`Alt+F4` 照常可用。启动时窗口在工作区居中。
- **流畅度**：动画进行时按显示器刷新节奏（DWM）逐帧绘制，空闲时回到普通消息等待；启动动画的蓝图底板与网格预先缓存，收缩阶段只做位图合成，界面在标志停顿时预先绘制一次，不再每帧重绘。
- **省电**：动画只在需要时以定时器驱动，静止后自动停止；后台任务的指示动画仅在启动器窗口处于前台且未最小化时播放，游戏运行期间不会占用绘制资源。
- **高 DPI**：所有绘制以 96 DPI 逻辑坐标设计，按系统 DPI 缩放。

界面预览见 `docs/blueprint-*.png`。

## 标志与图标

`assets/logo/` 保存矢量标志：`offset-mark.svg`（浅色）、`offset-mark-dark.svg`（深色）、`offset-icon.svg`（应用图标）、`offset-icon-1024.png` 与多尺寸 `offset.ico`（16–256 px）。图形按原始图片精确重建：五条交织的直线带与同心齿轮，非描摹位图。启动器内的标志由同一几何生成的 `src/logo_data.inc` 以矢量绘制。替换 `assets/logo/offset.ico` 后运行 `tools/make-icon.ps1` 生成 `src/icons/` 中嵌入 EXE 的单尺寸图标，再重新构建。

## 多版本与加载器

下载任务按顺序执行，最多保留 64 项；同一组合不会重复添加。可以混合安排不同原版和不同加载器任务，失败项不会阻止后续任务，可取消或重试。移除任务、清空完成项只整理列表，不删除已下载的游戏。任务列表保存在本次进程内存中，重新打开后可利用已校验的缓存继续安装。

下载时可以点击侧栏或使用 Ctrl+1–Ctrl+9 切换页面，任务继续在后台运行；安装参数和文件操作在任务结束后恢复。底部显示本次任务的实际下载量与已完成下载文件数，包含重试传输的数据，命中缓存的文件不计入传输量；该数字不是整个安装流程的百分比。进度动效跟随显示刷新，最小化或切到其他应用时停止装饰动画。

启动时读取版本元数据中的 `javaVersion.majorVersion`。如果当前 Java 版本不足，启动器会在 `C:\Program Files\Java\jdk-*` 中选择满足要求的最高已安装 JDK，并只对本次启动使用；若没有可用版本，会显示所需主版本并打开 Oracle 对应版本的官方下载页。Java 路径不会被静默写回用户设置。

下载期间可切换右侧标签、编辑其他下载中心的搜索条件、滚动已有结果及调整外观；新的联网任务、安装和启动仍按现有单任务顺序执行。模组中心和整合包首次进入浏览页时，自动执行空关键词搜索，按当前筛选条件分页显示默认结果。已有后台任务时延后到空闲加载；手动输入的关键词不被覆盖，空结果或失败不会因界面重绘而反复请求。实际下载量显示在资源状态下一行。

Fabric / Quilt 的兼容列表与配置来自各自官方 Meta 服务。Forge / NeoForge 下载并校验官方安装器，再使用其无界面客户端安装模式执行处理器。安装日志保留在游戏目录的 `.offset-install`，不会用空配置覆盖已有 `launcher_profiles.json`；官方安装器会更新对应条目，首次使用时只在缺失时创建最小配置。

加载器配置与基础版本合并为可独立启动的现代元数据；原始配置保存在版本目录的 `offset-original.json`，已有备份不会被覆盖。重复安装前的配置另存为 `offset-before-install.json`。客户端优先创建硬链接，不支持时复制；加载器各版本分别准备本地依赖目录。

兼容列表由服务实际返回决定。NeoForge 支持已核实的 1.20.2 及以后的 1.x 正式版与 26.x 稳定版本映射；1.20.1 旧坐标和未知快照映射暂不支持。没有对应加载器时会显示“暂无兼容版本”。每个游戏版本仍需匹配合适的 Java，Java 不随启动器安装。

## 每版本管理

0.6 可从首页的“管理”、版本管理“已安装”页的“管理”，或双击已安装版本进入该版本页面。打开页面不会切换首页当前版本；“使用该版本”和“启动”会明确选用它。

- **概览**：查看基础游戏、加载器、实际数据目录和生效的 Java / 内存设置，启动或补全此版本，并打开数据、版本、模组、存档、日志和资源包目录。
- **设置**：设置显示名、Java、最大内存和游戏目录隔离。显示名只用于界面，不更改版本 ID 或目录；Java 留空继承全局设置；内存填 `0` 继承全局设置，否则需在 `512 MiB` 到本机物理内存总量之间。
- **模组**：管理此版本实际数据目录中的模组，支持刷新、启用、禁用、移入回收区和打开目录。“下载模组”会选用该版本并进入模组中心。
- **加载器**：查看当前加载器版本，点击“刷新版本”，选择目标后升级或降级。支持 Fabric、Forge、NeoForge 和 Quilt 的同一加载器版本更换，Minecraft 基础版本保持不变；原版或无法安全识别的自定义配置不提供此操作。模组本身不会自动更新，仍需与目标加载器兼容。

默认使用共享游戏目录；可为单个版本改为独立的 `游戏根目录/versions/<版本 ID>/game`，使其模组、存档和配置使用该目录。切换隔离不会自动移动、复制或删除原有数据，需要自行迁移所需文件。原版客户端、`libraries` 和 `assets` 缓存仍使用游戏根目录，不因隔离重复下载。

各版本设置保存在 `游戏根目录/versions/<版本 ID>/.offset-profile.ini`，由游戏根目录与版本 ID 共同区分；不会覆盖全局 Java / 内存设置，也不修改原版或加载器 JSON。

加载器更换保持原版本 ID、显示名、Java、内存、隔离目录及模组／配置／存档文件。启动元数据会按基础游戏与目标加载器的官方配置重新生成，额外手改的启动 JSON 参数不合并。Forge／NeoForge 使用该版本生效的 Java 执行官方安装器。

准备工作在游戏目录的 `.offset-loader-change/<任务 ID>` 完成，日志及缓存也保留在那里。校验通过后才以一次原子替换发布启动 JSON；已有共享支持库只有校验匹配时才复用，遇到冲突会保留原文件并停止更换。每次生成独立的本地依赖目录及 `offset-loader-before-*.json` 历史快照，“恢复上次加载器”可切回上次配置。失败或取消保留原启动配置；历史文件和仍被配置引用的本地依赖目录应一起保留。


## 服务器

侧栏“服务器”（`Ctrl+9`）读取当前选用版本的 `servers.dat`，显示 Minecraft 多人游戏界面中保存的名称、地址和在线人数／人数上限。共享版本读取游戏根目录，隔离版本读取其 `versions/<版本 ID>/game`；服务器列表不会被修改。

首次进入或切换游戏目录／版本后自动查询，点击“刷新”可重新读取及查询。查询使用 Java 版服务器状态协议，未获得有效人数时显示离线、无法查询或地址无效，不以 `0 / 0` 冒充结果；服务器返回的人数是其公布的状态快照。此页面不登录服务器，也不发送账户令牌。

支持域名、IPv4、IPv6 和显式端口；未指定端口的域名会查询 Minecraft DNS SRV 记录。最多读取 256 条，文件最大 16 MiB；不支持的损坏或超限列表会明确提示。最多同时查询 8 台，每台的连接与收发预算为 5 秒；DNS 使用 Windows 系统超时，取消在 DNS 返回后生效，连接及收发期间每 50 毫秒检查取消。仅支持 Java 版现代服务器状态协议，未提供基岩版或早期旧式 Ping 回退。

## 身份模式

**离线模式**：填写 3–16 位 ASCII 字母、数字或下划线组成的玩家名。离线 UUID 按 Java 版的 `OfflinePlayer:<玩家名>` 规则生成，适用于单人游戏和允许离线身份的服务器。需要正版认证的服务器仍要求有效在线身份。

**微软账户**：在“账户”页点击“登录”，按设备代码在微软网页完成授权，程序依次验证 Xbox Live、XSTS、Minecraft 权益和 Java 版档案。已预置 OFFSET 的 Application / Client ID：`54048348-72bb-4ed2-a8a4-c72423265e94`，通常无需填写；“高级”中可改用自己的注册应用。启动器不收集微软密码。

新应用需要完成以下配置：

1. [在 Microsoft Entra 注册应用](https://learn.microsoft.com/en-us/entra/identity-platform/quickstart-register-app)，选择支持个人 Microsoft 账户的账户类型，复制 Application（client）ID。
2. 在 Authentication 中启用 **Allow public client flows**，参见[微软桌面应用配置说明](https://learn.microsoft.com/en-us/entra/identity-platform/scenario-desktop-app-configuration#enable-public-client-flow)。本程序使用[设备代码授权流程](https://learn.microsoft.com/en-us/entra/identity-platform/v2-oauth2-device-code)，请求 `XboxLive.signin offline_access`，个人账户使用 `consumers` 授权地址。
3. 根据 [Minecraft AppID 审核入口](https://aka.ms/mce-reviewappid)申请相应服务权限。注册应用本身不代表已获准调用 Minecraft API；未获准的 ID 可能在 Minecraft 认证阶段收到拒绝响应。

项目不内置其他启动器的应用 ID。刷新凭据使用当前 Windows 用户的 DPAPI 加密，原子保存到 EXE 旁 `offset.account`，不写入 INI 或日志；此文件不能跨用户或机器直接复用。启动时恢复会话，临近过期时自动续期；在线启动前再次检查会话，失败时停止启动并显示原因。Minecraft 访问令牌只保存在内存中。取消或失败的新登录保留原有加密账户；“退出登录”清除当前身份和本地加密凭据。

## 角色皮肤

“账户 → 皮肤”提供当前正版皮肤与本地 PNG 的正反面预览，支持经典／纤细模型和透明外层。选择文件只预览，点击“上传应用”后才修改当前登录账户。支持不超过 1 MiB 的完整 `64×64` 或旧式 `64×32` PNG；旧式皮肤按经典模型镜像补齐四肢。

“刷新”重新读取官方皮肤，“恢复默认”移除该账户的自定义皮肤，“导出当前”保存已获取的当前皮肤 PNG。上传和恢复默认需要有效在线会话，失败不自动重放；服务端已成功但预览下载失败时会单独提示。纹理仅从官方 Minecraft 纹理域名经 HTTPS 下载，不向纹理服务器发送账户令牌。本地预览在退出时清理，不会写入整合包。

## 模组中心

0.2 增加“模组中心”，使用 [Modrinth](https://docs.modrinth.com/api/operations/searchprojects/) 官方来源，沿用原生界面与汇编下载实现。

下载模组：

1. 打开“模组中心 → 下载模组”，填写 Minecraft 版本并选择 Fabric、Forge、NeoForge 或 Quilt。
2. 输入名称或关键词后搜索；首次进入会显示 Modrinth 默认结果。使用“上一页 / 下一页”浏览，每页最多 20 个项目。
3. 选择左侧项目，在右侧选择兼容文件，再点击下载。列表最多显示 32 个匹配文件；更改游戏版本或加载器会清空旧结果。
4. 文件下载到当前版本实际数据目录的 `mods` 文件夹：共享模式使用游戏根目录，隔离模式使用 `versions/<版本 ID>/game`。若文件声明了必需依赖，可点击“查看必需依赖详情”查看原来源的依赖项目或版本 ID，再另行安装；本版不自动安装依赖。

下载先写入该数据目录下的 `.offset-mod-cache`，通过官方 SHA-1 和 JAR/ZIP 文件头检查后再移入同目录下的 `mods`。扫描、启停、移入回收区及打开目录也使用同一版本数据目录，管理页面会显示实际路径。同名且内容不同的文件不会被覆盖；内容相同时直接复用。API 没有提供可用下载地址或校验值的文件会显示“需从官网获取”，点击后打开项目官网，不拼接下载地址。

“本地管理”扫描 `mods` 中的 `.jar` 和 `.jar.disabled` 文件，可刷新、启用、禁用、移入回收区或打开模组目录。禁用会追加 `.disabled`，启用会恢复 `.jar`；发生同名冲突时保留原文件。回收区位于 `mods/.offset-trash`，可以手动将文件移回 `mods` 并恢复原文件名。列表最多显示 512 个本地模组。

加载器可在“版本管理”中安装。切换当前已安装版本后，模组中心会同步其基础游戏版本及识别出的加载器；同一版本下手动调整的筛选会保留。请确认模组的游戏版本、加载器和依赖与所选游戏一致。

## 整合包

“整合包 → 下载整合包”使用 Modrinth 搜索；首次进入会显示默认结果，Minecraft 版本筛选可以留空。选择左侧项目，在右侧选版本，点击“下载安装”。

“导入整合包”标签和“导入文件”按钮可选择本地 `.mrpack` / `.zip`，按包内清单识别格式，支持：

- **Modrinth**：`modrinth.index.json`，含公共配置与客户端覆盖层。
- **CurseForge**：标准 `manifest.json` 导出包及其 overrides 目录。
- **Prism Launcher / MultiMC**：`mmc-pack.json`、`instance.cfg` 和 `.minecraft/` 或 `minecraft/` 游戏数据目录。

清单可在压缩包根目录，或位于一个共同的顶层包装目录中。选择文件后先在后台预检，显示格式、包名、Minecraft / 加载器和实例目录名；预检不安装、下载或创建实例。可修改 ASCII 实例目录名，再点击“导入”。确认时再次校验预览所绑定的归档 SHA-1，文件被替换或修改需重新选择。取消文件选择保留原预览，预览页的“取消导入”只清除预览。包名沿用清单中的显示名，实例目录名不会重命名原归档。

程序下载并验证清单中的客户端文件，准备基础游戏、Fabric / Forge / NeoForge / Quilt 加载器及原生依赖，复制整合包配置。完成后在“已安装”中选择包，点击“选用整合包”切换到其游戏目录，再点击首页启动按钮。Java 仍需自行安装，且必须符合该包要求。

整合包实例位于 EXE 旁的 `instances/`，下载归档保存在 `downloads/packs/`。每个实例的模组、配置和存档相互独立。安装先写入专属 `.offset-stage` 临时目录，成功后才发布；取消或失败保留缓存，同一归档可重试，已有完整实例不会被覆盖。解压前验证归档路径、条目类型、重复名称、大小与清单路径。

Modrinth 的客户端 optional 文件默认包含，客户端不支持的文件跳过。必需文件被作者限制分发时会停止并提示官网获取，不推测下载地址。

Prism / MultiMC 导入接受现代 Minecraft 和 Fabric / Forge / NeoForge / Quilt 标准组件，通过官方元数据重建游戏与加载器；作为依赖声明的 LWJGL / intermediary 也由此重建。已禁用的组件、自定义组件、运行时版本、`patches/`、`jarmods/` 或非空启动前后命令 / Wrapper 命令会明确拒绝。不会执行归档内脚本，也不会迁移其他启动器的 Java 路径、JVM 参数或账号设置。含两套游戏数据目录的歧义包会拒绝；其他平台的专有安装脚本或服务器安装包不属于通用 ZIP 导入支持范围。

当前支持普通 store/deflate ZIP，归档上限 512 MiB、4096 项，单项解压上限 128 MiB、解压总量上限 2,147,483,647 字节（约 2 GiB）；不支持 ZIP64、加密或链接条目。

### 导出整合包

“整合包 → 导出整合包”参考 [PCL-CE 的导出流程](https://github.com/PCL-Community/PCL-CE/blob/1a7653127d578efdb2a9388999a3fdc67d9e49d5/Plain%20Craft%20Launcher%202/Pages/PageInstance/PageInstanceExport.xaml)：选择当前游戏目录中的已安装版本，填写包名、版本号和作者，选择内容、预览、选择位置，最后点击“导出”。版本号默认为 `1.0.0`；来源下拉框独立于首页当前启动版本，并显示该版本实际数据目录。也可从“版本管理 → 所选版本 → 概览 → 导出整合包”直达，并预选该版本；打开导出不会切换首页启动版本。

模组、配置、资源三组默认包含，存档默认不包含。配置组含 `config`、`defaultconfigs`、`kubejs`、`scripts` 和受支持的 `options*.txt`；资源组含 `resources`、`resourcepacks`、`shaderpacks` 和 `datapacks`。已知账号与凭据文件名、日志、缓存、游戏本体及依赖库、截图和服务器列表不打包。这里提供分类开关，不提供逐文件规则编辑或打包启动器。

输出可选 **Modrinth `.mrpack`、CurseForge `.zip`、Prism / MultiMC `.zip`**。三种格式都直接携带所选本地内容：前两种写入标准清单和 `overrides/`，Prism 写入标准组件及 `.minecraft/`。不伪造下载地址或 CurseForge 文件 ID，不上传平台，也不包含 Minecraft、Java 或启动器程序；基础游戏和加载器在导入时按清单重建。

预览只读取来源，显示文件数、大小和部分相对路径；更换来源或内容分类后需重新预览。确认后后台重新扫描并生成新文件，可取消；成功后可打开导出目录。输出必须在源游戏目录外且不存在，不覆盖同名文件、不更改源实例。原创汇编 ZIP writer 使用 store 条目，无新增运行依赖；导出归档最多 512 MiB、单文件最多 128 MiB、所选内容最多 4093 个文件。

## 启动与崩溃日志

侧栏“日志”或 `Ctrl+8` 打开原生只读日志页，可查看启动输出或崩溃报告、打开文件目录并导出当前页的日志 / 报告。界面预览最多显示末尾 256 KiB，导出保留完整的已保存文件。异常退出或 Java 启动失败后自动打开本次报告；正常退出也保留启动输出。日志记录本次版本、有效 Java / 内存 / 数据目录、进程退出码或 Windows 启动错误码，不记录完整会话命令行。

日志位于实际数据目录的 `.offset-logs/`，每次运行使用带日期、时间和序号的独立标识，启动输出为 `launch-<标识>.log`，异常报告为同标识的 `launch-<标识>.txt`；先写临时文件，保存成功后发布。Java 标准输出和错误输出使用 UTF-8 收集，保留原有文件字符集设置；异常报告可附带实际数据目录的 `logs/latest.log`、`crash-reports/` 中最近的本次报告以及 `hs_err_pid*.log`。附件按本次启动时间和文件修改时间筛选，避免把旧崩溃当成本次结果。文本为 UTF-8；每个日志 / 报告最多 4 MiB，Java 输出最多 3 MiB，每份附件最多收录最后 1 MiB，超出会标记截断。

生成日志时会替换本次已知的 Minecraft 认证令牌和本地敏感值，并处理跨读取边界的匹配；这不等于能自动识别任意模组写出的所有私人信息。分享前可在日志页查看内容。导出接受 `.txt` / `.log`，覆盖已有目标需通过保存对话框确认；日志写入失败会保留错误状态，不会把未保存的报告显示为已生成。

## 模组介绍与 MC 百科

在模组或整合包搜索中选中项目，点击“介绍”，可滚动阅读完整正文及来源、下载数、许可或作者等元信息。介绍在原生文本控件中显示，保留段落，HTML 和 Markdown 转为可读文字；图片、复杂排版及外部链接可通过“项目官网”查看。

“MC 百科”会以项目名称在官方 `search.mcmod.cn` 搜索；它是百科检索入口，不会把名称相似的项目冒充为已验证对应条目。详情页可返回原来的搜索与版本列表。

## 个性化

侧栏“个性化”提供六种预设主题色，也可输入六位 HEX（可带 `#`）实时预览。主题覆盖按钮、导航、焦点边框和首页蓝图色调，过浅颜色会自动加深以保持白字可读。点击“应用”保存，或“恢复默认”还原。

可以修改窗口与侧栏显示名称，以及开启或关闭界面动效。个性化选项保存在 `offset.ini`：`ThemeColor`、`Animations` 和兼容 Unicode 的 `LauncherNameHex`；重新启动和切换语言后保持生效。原有三语言、留白布局、标志与蓝图风格均保留。

## 支持范围

当前安装与启动实现面向 **使用现代启动元数据的 Minecraft 版本，通常为 1.13 及以后**。下载目录列出官方正式版和快照，旧式启动元数据会明确报错。支持基础游戏及上述加载器配置、本地版本选择、下载与补全、Windows 依赖规则、资源校验、离线 UUID、微软登录和 Java 参数生成。具体游戏版本仍需匹配合适的 Java。

已实测 **Minecraft 1.21.1 + Java 21**。参数文件要求 Java 9 或更高版本，因此不支持直接使用 Java 8 启动；早期游戏版本需另行验证兼容性。当前将演示、快速启动及自定义分辨率功能视为关闭，不匹配 `os.version` 正则条件。

下载器生成的加载器配置会先完成继承合并后再供启动；手动放入而未合并的 `inheritsFrom` 配置不会冒充已完成的安装。旧版启动参数、Java 自动下载和后台自动更新尚未实现。

普通版本默认共享同一游戏根目录中的存档、模组和配置；开启该版本隔离后，使用其 `versions/<版本 ID>/game`。通过整合包页安装的包使用各自独立的实例根目录。

## 快捷键

| 快捷键 | 操作 |
| --- | --- |
| `Ctrl+1` / `Ctrl+2` / `Ctrl+3` / `Ctrl+4` | 开始游戏 / 版本管理 / 账户 / 设置 |
| `Ctrl+5` | 模组中心 |
| `Ctrl+6` / `Ctrl+7` | 整合包 / 个性化 |
| `Ctrl+8` | 日志 |
| `F5` | 每版本管理页：启动该页版本；其他页面：启动当前版本 |
| `F6` | 每版本管理页：补全该页版本；版本下载页：添加所选项并启动任务；其他页面：补全当前版本 |
| `F7` | 在“下载模组”页搜索当前条件 |
| 搜索关键词框中的 `Enter` | 搜索模组 |
| `Ctrl+S` | 每版本管理页：保存该版本设置；其他页面：保存全局设置 |
| `Esc` | 请求取消当前下载或登录任务 |

## 构建与验证

在项目根目录打开 PowerShell：

```powershell
.\build.ps1
.\build.ps1 -Run
.\test.ps1
.\test.ps1 -Network
.\test.ps1 -Network -JavaProbe -JavaHome 'C:\Program Files\Java\你的JDK目录'
.\package.ps1
```

构建工具是随项目提供的 **FASM 1.73.32**，位于 `tools/FASM.EXE`，头文件位于 `tools/INCLUDE/`。如需重新获取，可从 [FASM 官方站点](https://flatassembler.net/download.php)下载并按相同目录结构放置。

`build.ps1` 先调用 `tools/gen-strings.ps1`，把 `src/strings.json` 中的界面文案（`ui`，UTF-16）与后台状态文案（`status`，UTF-8）按 zh / en / ja 生成 `src/strings.inc` 与 `src/strings_status.inc`，再组装生成 `dist/OFFSET.exe`。新增或修改文案时三种语言需同时填写，且 `%u` / `%s` 占位符保持一致。PowerShell 仅用于构建和验证，启动器运行时不依赖 PowerShell、.NET、Python 或 Node.js。

默认测试覆盖 JSON 解析、Unicode、哈希、物理内存检测及边界、身份生成、参数处理以及模拟的微软登录流程；`-Network` 额外进行官方服务联网及 8 线程状态隔离检查；`-JavaProbe` 启动真实 Java 进程检查中文目录、空格、引号和身份参数，需要安装 JDK。测试程序及缓存放在 `tests/` 下，不属于便携分发必需文件。资源下载最多使用 8 个后台工作线程，可取消并复用已经校验过的文件。

下载源测试覆盖域名边界、路径映射、镜像失败与校验失败后的回退、取消，以及镜像清单过期或无效时查询官方。`-Network` 还通过原生下载实现获取 Minecraft 1.21.1 的版本配置、客户端、支持库、资源索引和一个资源文件，并逐项校验 SHA-1。

模组默认测试覆盖 Modrinth JSON 解析、兼容性筛选、下载受限处理及本地文件管理。`-Network` 还运行真实 Modrinth 搜索、版本读取、Fabric API 的 Minecraft 1.21.1 文件下载、SHA-1 校验和启用/禁用验证。测试使用隔离的 `tests/out` 目录。

0.7 增加 `pack-import`、`pack-import-ui`、`launch-log` 和 `logs-ui` 测试，覆盖清单识别、包装目录、预检与确认分离、预览哈希绑定、目录输入与冲突、输出捕获、当次附件、脱敏、导出和失败处理。真实 Java 日志探针检查标准输出 / 错误输出、异常退出及启动失败；编译、探针、GUI 和实际 Minecraft 运行结果分别记录于 `docs/validation.json`，测试入口不代表所有检查均已完成。

`pack-export-ui` 原生控件测试已验证独立来源选择、草稿保留、只读文件预览、分类默认值与失效处理、选择位置取消、同名文件保护、UTF-8 元信息、明确确认及忙碌状态。`pack-export` / `zip-writer` 分别覆盖整合包生成和 ZIP writer。三种格式已通过真实归档生成、CRC32、UTF-8 文件名与六次导出再导入校验；模组、配置、资源分类和可选存档均已检查，源文件哈希保持不变。Windows 界面已完成 Modrinth 导出，导入的 Prism 实例已实际进入 Minecraft 1.21.1 + Fabric 主菜单并正常退出，详见 `docs/validation.json`。

此前 0.6 新增 `version-settings`、`version-page`、`version-mods` 和 `version-actions` 原生测试，覆盖按根目录与版本 ID 分离的设置、继承与内存边界、页面上下文、隔离模组目录、冲突保留，以及选用或补全版本后的上下文恢复。`-JavaProbe` 另运行 `version-launch`：已通过真实 JVM 进程验证每版本 Java / 内存、共享与独立工作目录、根目录支持库 / 资源缓存和全局设置恢复。Windows 界面已验证版本入口、独立保存、重启保留、版本间设置隔离、模组启停，以及中文 / English / 日本語切换；Minecraft 1.21.1 + Fabric 0.19.5 + Fabric API 0.116.12 已使用该版本独立目录和 2048 MiB 内存进入主菜单并正常退出，全局 4096 MiB 设置保持不变。预览见 `docs/version-management-*-0.6.jpg`，运行画面见 `docs/version-isolated-runtime-0.6.jpg`。

此前版本已完成实际 JVM 进程探针验证、官方 Minecraft 1.21.1 全量安装、离线启动及主菜单实际渲染检查，并通过游戏菜单正常退出。认证状态机测试使用真实 JSON 解析器与模拟响应，**尚未用已获准的应用 ID 和真实微软账户完成登录验证**。这些结果不代表所有 Minecraft 版本均已实测。

0.5 已在 Windows 上通过完整原生测试、真实 Modrinth 整合包检索和下载、介绍读取及 Java 参数探针；Fabulously Optimized 6.5.0 / Minecraft 1.21.1 已完成独立实例安装，50 个客户端文件通过 SHA-1 与 SHA-512 复核，63 项配置文件通过字节复核，并实际进入带模组的游戏主菜单、正常退出。

0.3 的新增验证覆盖正式版 1.21.1 与快照 24w33a 的界面批量下载；Fabric 0.19.5、Forge 52.1.20、NeoForge 21.1.264、Quilt 0.30.1 均完成真实安装，并在 Minecraft 1.21.1 / Java 21 下进入主菜单、正常退出。具体证据见 `docs/validation.json`。

上述游戏验证缓存保留在 `tests/out/vanilla-smoke`，未放入便携包；在设置中使用该目录及 Java 21 可直接复测。构建和验证记录位于 `docs/validation.json`。


0.8 增加真实 Windows DPAPI 缓存测试（使用隔离测试凭据），覆盖客户端绑定、轮换、到期刷新、取消保留旧缓存及退出；在线启动门禁测试确认失效会话不会创建 JVM。皮肤接口测试使用模拟 HTTP，不修改真实账户；PNG 和界面测试实际运行 GDI+，检查正反面、透明层、纤细模型、旧版镜像及源文件锁释放。OFFSET 的 Client ID 已通过真实微软设备代码接口验证；真实账户授权、Minecraft API 批准和皮肤变更仍需持有者登录验证。

0.9 增加 `loader-change`、`loader-change-ui`、`servers-nbt`、`servers-net` 和 `servers-ui`。加载器事务测试检查升降级、回退、文件冲突、取消及源数据保留；另在 Minecraft 1.21.1 / Java 21 下使用官方服务完成 Fabric、Forge、NeoForge、Quilt 的真实升降级和恢复，但本轮没有启动游戏。服务器测试使用真实二进制 NBT、原生列表控件及本地 TCP 状态服务，覆盖 Unicode、共享／隔离目录、上限、损坏数据、IPv4／IPv6、SRV、碎片响应、超时及取消，不依赖用户的服务器列表。验证细节见 `docs/validation.json`。

## 源码

| 路径 | 内容 |
| --- | --- |
| `src/main.asm` | 窗口、页面布局、设置、快捷键与工作线程 |
| `src/ui_gfx.inc` | GDI+ 抗锯齿图形、蓝图底纹、弹簧与转场动画、悬停区域 |
| `src/i18n.inc` | 界面语言切换、状态文案翻译与字体选择 |
| `src/logo_data.inc` | 标志矢量坐标（由 `assets/logo` 几何生成） |
| `src/engine.inc` | 版本安装、文件校验、身份及 Java 启动参数 |
| `src/mc_sources.inc` | 原版下载的 BMCLAPI 地址映射与官方回退 |
| `src/versions.inc` | 正式版、快照目录与多版本安装队列 |
| `src/versions_ui.inc` | 下载、已安装与任务列表的原生界面 |
| `src/version_settings.inc` | 每版本设置持久化、继承、隔离目录解析与启动覆盖 |
| `src/version_page.inc` | 每版本概览、设置和目录入口的原生界面 |
| `src/version_mods.inc` | 每版本模组管理与工作线程目录上下文 |
| `src/version_actions.inc` | 当前版本摘要、显式选用、启动与补全调度 |
| `src/loaders.inc` | Fabric、Forge、NeoForge、Quilt 查询与官方安装器调用 |
| `src/loader_change.inc` | 加载器升降级、校验缓存、原子发布和配置回退 |
| `src/loader_change_ui.inc` | 每版本加载器列表、升级／降级及恢复操作 |
| `src/servers_nbt.inc` | Minecraft 已保存服务器 NBT 读取 |
| `src/servers_net.inc` | 原生 TCP 状态查询与在线人数解析 |
| `src/servers_ui.inc` | 服务器列表与刷新界面 |
| `src/profiles.inc` | 加载器继承合并、Maven 依赖及独立启动配置 |
| `src/auth.inc` | Microsoft → Xbox → XSTS → Minecraft 认证 |
| `src/skin.inc` | 官方角色皮肤读取、PNG 校验、上传、重置与导出 |
| `src/account_ui.inc` | 账户、设备代码操作与像素皮肤预览 |
| `src/common.inc` | WinHTTP 请求、流式下载、JSON、SHA-1 与目录处理 |
| `src/memory.inc` | 总物理内存检测、MiB 上限与输入验证 |
| `src/mods_ui.inc` | 模组中心页面、搜索与文件选择、操作调度 |
| `src/catalog.inc` | 整合包来源搜索、兼容版本和校验下载 |
| `src/packs.inc` | 归档验证、清单安装和独立实例发布 |
| `src/pack_import.inc` | 整合包只读预检、包装目录与 Prism / MultiMC 格式解析 |
| `src/packs_ui.inc` | 整合包下载、导入及已安装页面 |
| `src/pack_export.inc` | 按版本有效数据目录预览、自包含清单与整合包导出 |
| `src/pack_export_zip.inc` | 原创有界 ZIP/store writer、CRC32 与原子发布所需输出 |
| `src/pack_export_ui.inc` | 独立来源选择、分类开关、预览与确认导出页面 |
| `src/launch_log.inc` | 原生启动输出、崩溃报告、脱敏与日志导出 |
| `src/logs_ui.inc` | 启动输出和崩溃报告的原生阅读界面 |
| `src/project_details.inc` | 项目介绍解析、文本转换和百科搜索地址 |
| `src/details_ui.inc` | 滚动介绍阅读界面 |
| `src/personalize.inc` | 主题色、名称、动效及配置持久化 |
| `src/splash.inc` | 启动动画：蓝图底板收缩并把标志移交到首页大标志 |
| `src/sidebar.inc` | 侧栏收起 / 展开、内容区平移与收起状态提示 |
| `src/mod_profile.inc` | 当前游戏配置与模组筛选同步 |
| `src/mods.inc` | 模组数据、每版本目录路由、本地管理、下载校验、DPAPI 密钥保存 |
| `src/modrinth.inc` | Modrinth 官方搜索与兼容文件查询 |
| `src/mod_types.inc` | 模组项目、文件与本地列表的容量和结构定义 |
| `src/imports.inc` | Windows API 导入 |
| `src/strings.json` | 中 / 英 / 日文案；构建时生成 `strings.inc`、`strings_status.inc` |
| `src/icons/` | 嵌入 EXE 的 16–256 px 图标 |
| `src/app.manifest` | Windows 应用清单 |
| `tests/` | 原生测试程序与验证用例 |

JVM 参数及主类使用参数文件，游戏身份参数经 Windows 命令行规则转义后传给 Java。登录令牌不写入 JVM 参数文件；认证请求禁止自动重定向，下载文件在校验完成后才替换目标缓存。

## 设计参考与许可

功能组织参考 [Prism Launcher](https://github.com/PrismLauncher/PrismLauncher)、[PCL-CE](https://github.com/PCL-community/PCL-CE) 和 [HMCL](https://github.com/HMCL-dev/HMCL) 的版本、账户与启动配置理念。启动器代码和界面绘制为本项目原创，未复制这些项目的代码或美术资源，也与这些项目及 Mojang / Microsoft 无隶属关系。

原创代码使用 MIT 许可，见 `LICENSE`。随附 FASM 工具遵循其自身许可，见 `tools/LICENSE.TXT`。Minecraft 游戏文件由用户按需从官方服务器获取，遵循其各自许可。
