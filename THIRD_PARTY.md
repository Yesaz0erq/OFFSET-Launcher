# Third-party tools and services

OFFSET runtime source and vector artwork are original. No launcher source code or artwork from PCL-CE, HMCL, or Prism Launcher is included. Their public projects informed feature organization and service URL conventions.

The build tool is flat assembler 1.73.32 by Tomasz Grysztar, downloaded from <https://flatassembler.net/fasmw17332.zip>. The compiler, Windows headers, and macros in `tools/` retain their original license in `tools/LICENSE.TXT`; the project MIT license does not replace that license. The compiler is a build-time dependency and is not bundled into OFFSET.exe.

The launcher dynamically calls Windows system DLLs, including GDI+ for its original vector interface. For versions using native-library classifiers, it invokes the Windows-provided tar.exe to extract verified archives; it also extracts version metadata from downloaded loader installers and modpack archives after independent ZIP validation. Neither Windows nor Java is redistributed here. A compatible Java runtime must be installed and selected separately for Minecraft and Java-based installer execution.

Minecraft metadata and game resources are downloaded separately when requested, using BMCLAPI mirror endpoints with Mojang official endpoints as fallback. Public URL layouts were checked against PCL-CE; source routing is implemented independently in assembly. BMCLAPI documentation: <https://bmclapidoc.bangbang93.com/>. Game resources and the Minecraft name remain the property of their respective owners. OFFSET is an independent launcher and is not an official Minecraft product.

Version 0.3 adds release/snapshot browsing, a multi-version installation queue and loader-profile normalization through original assembly modules (`version_types.inc`, `versions.inc`, `versions_ui.inc`, `loaders.inc`, `profiles.inc` and `mod_profile.inc`). Third-party metadata, libraries and installers are downloaded into the user's game/cache directories on demand; they are not embedded in OFFSET.exe or the portable release package.

Loader services and artifacts:

- Fabric profiles and versions come from [Fabric Meta](https://meta.fabricmc.net/); loader libraries come from the repositories declared in its profile, including [Fabric Maven](https://maven.fabricmc.net/).
- Quilt profiles and versions come from [Quilt Meta](https://meta.quiltmc.org/); loader libraries come from its declared repositories, including [Quilt Maven](https://maven.quiltmc.org/repository/release/).
- Forge version metadata comes from [Forge files](https://files.minecraftforge.net/), and the official installer JAR/checksum comes from [Forge Maven](https://maven.minecraftforge.net/). A BMCLAPI mirror can supply the same installer bytes, checked against the official checksum. The downloaded [official Forge installer](https://github.com/MinecraftForge/Installer) is run with `--installClient` using the selected Java runtime.
- NeoForge versions and official installer JARs/checksums come from [NeoForge Maven](https://maven.neoforged.net/). The downloaded [official NeoForge installer](https://github.com/neoforged/Installer) is run with `--installClient` using the selected Java runtime.

These projects and their artifacts retain their own licenses and notices; OFFSET's MIT license applies to its original code, not to Minecraft, loader installers or downloaded libraries. Installing a loader may run its official installer processors to create the client-side artifacts required by that loader. Normalizing their published profile format does not copy their launcher or installer source into OFFSET.

The mod browser separately uses the official [Modrinth API](https://docs.modrinth.com/) and [CurseForge API](https://docs.curseforge.com/rest-api/), together with the download URLs permitted by those services. CurseForge requires a user-supplied API key; mod download availability remains subject to the service response and author distribution settings.

Version 0.5 adds original assembly modules for a modpack catalogue and independent-instance installation (`catalog.inc`, `packs.inc`, `packs_ui.inc`), native project introductions (`project_details.inc`, `details_ui.inc`), and launcher personalization (`personalize.inc`). Modrinth packs follow its published [`.mrpack` specification](https://support.modrinth.com/en/articles/8802351-modrinth-modpack-format-mrpack), including the root index, client files and override layers. CurseForge packs use the [exported ZIP profile structure](https://support.curseforge.com/support/solutions/articles/9000197912-exporting-and-importing-modpacks), resolving project/file IDs through the [official API](https://docs.curseforge.com/rest-api/). Downloaded packs, mods, resource packs, shaders and their descriptions retain their respective authors' licenses and distribution terms; they are not included in OFFSET.exe or the portable package.

Project descriptions are retrieved from the selected service and converted to plain text for a native read-only view; their authorship remains with the original project. The original project website remains available for rich content. The explicit MC百科 action uses the official [MC百科 website](https://www.mcmod.cn/) and its [search service](https://search.mcmod.cn/) with an encoded project-title query. It does not copy MC百科 articles, claim an exact article mapping, or imply endorsement by MC百科. The theme palette and personalized display-name functions are original launcher code and do not redistribute PCL artwork, source or theme resources.

Version 0.6 adds original per-version management, settings, explicit actions and mod-directory routing (`version_page.inc`, `version_settings.inc`, `version_actions.inc`, `version_mods.inc`). It follows the feature organization of a version workspace and per-version overrides; no reference-launcher code, user interface assets or configuration format is included.

Version 0.7 adds an original archive-inspection and import-preview workflow (`pack_import.inc` and the extended `packs_ui.inc`) for Modrinth, CurseForge and supported Prism Launcher / MultiMC exports. Prism / MultiMC compatibility reads their published `mmc-pack.json` component and `instance.cfg` data structure, using the [Prism import-format recognition](https://github.com/PrismLauncher/PrismLauncher/blob/develop/launcher/InstanceImportTask.cpp) and [Prism metadata project](https://github.com/PrismLauncher/meta-launcher) as format references. The parser and installation control flow are independently implemented in assembly. Game and standard loader components are rebuilt from their official metadata; unsupported patches, custom components and launch commands are rejected rather than executed. This is file-format compatibility, not redistribution of another launcher's implementation, accounts or bundled runtime.

The original `launch_log.inc` and `logs_ui.inc` modules capture a child process's standard output / error through Windows pipes and create local UTF-8 diagnostic files. They use Windows system APIs and introduce no logging framework or extra runtime dependency. Minecraft, Java and mod-authored diagnostic text retain their respective authorship. Reports replace the launcher's currently known authentication token and API key before publication, but are not claimed to identify all private text that third-party components might emit; exporting a report is an explicit local file action, not an upload service.

The original `pack_export.inc`, `pack_export_ui.inc` and `pack_export_zip.inc` modules add local self-contained export. The flow and category defaults were checked against PCL-CE's [instance-export page](https://github.com/PCL-Community/PCL-CE/blob/1a7653127d578efdb2a9388999a3fdc67d9e49d5/Plain%20Craft%20Launcher%202/Pages/PageInstance/PageInstanceExport.xaml) at commit `1a7653127d578efdb2a9388999a3fdc67d9e49d5`: pack metadata, category selection, local destination and background work. The controls, source scan, manifest generation and bounded ZIP/store writer are independently implemented in assembly. No PCL code, icons, advanced export rules or launcher executable is incorporated; three format choices are OFFSET's own feature scope.

Modrinth and CurseForge outputs use their documented manifest / overrides structures, while Prism / MultiMC uses its documented component / instance layout. All selected local content travels inside the archive, with no fabricated service download URLs or CurseForge file IDs and no platform-upload service. Minecraft and loader installation remains based on separately fetched official metadata. Export excludes known launcher account / credential files, logs and caches; optional user worlds and third-party mods/configuration retain their authorship and applicable terms. The ZIP writer uses Windows file APIs and an original CRC32 implementation, adds no compression library, and has no additional runtime dependency.

Reference projects: [Prism Launcher](https://github.com/PrismLauncher/PrismLauncher), [PCL-CE](https://github.com/PCL-community/PCL-CE), [HMCL](https://github.com/HMCL-dev/HMCL).

## flat assembler license


flat assembler  version 1.73
Copyright (c) 1999-2024, Tomasz Grysztar.
All rights reserved.

This program is free for commercial and non-commercial use as long as
the following conditions are adhered to.

Copyright remains Tomasz Grysztar, and as such any Copyright notices
in the code are not to be removed.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The licence and distribution terms for any publically available
version or derivative of this code cannot be changed. i.e. this code
cannot simply be copied and put under another distribution licence
(including the GNU Public Licence).

