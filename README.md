# Radium

This project is designed to solve cross-platform scenarios that require webview. Chromium Views + Chromium Content provide a complete set of capabilities. Mainly for large-scale application of WebUI.

Official explanation about WebUI: https://chromium.googlesource.com/chromium/src/+/main/docs/webui_explainer.md

This project is based on the Chromium open source project. Use //content + //ui and other required components.

# Supported Platforms

Planned to support Win/Mac/Linux/Android. Currently only Linux/Win/Mac platforms are supported.

# Demo

# Android Demo
[Android Demo](https://github.com/user-attachments/assets/504f4d84-204e-45a8-bd89-d66c979a5355)

## Win Demo (with dark mode)
[Win Demo](https://github.com/user-attachments/assets/9d9608b3-dc6c-4595-9761-3aacd8877d0d)

[Win Demo(dark mode)](https://github.com/user-attachments/assets/8a7ef266-d1a8-48dc-9179-a671ec6b6f4b)

## Mac Demo
[Mac Demo](https://github.com/user-attachments/assets/a5c45bf1-fb70-4a2b-9b62-03c0c4060a7f)

## Linux Demo
[Linux Demo](https://github.com/user-attachments/assets/3cc0ed85-7389-4d34-914c-9c34ab3f956c)

# Build
Pulling code requires the chromium depot_tools tool, please refer to: https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up

## Pull code:
```shell
mkdir radium && cd radium
gclient config --name "src/radium" --unmanaged https://github.com/liuwd8/radium.git
gclient sync --with_branch_heads --with_tags -D
# This will pull chromium and configure the chromium compilation environment for you
```

## Update
```shell
cd src/radium
git rebase origin/main
cd ..
gclient sync --with_branch_heads --with_tags -D
```

## Build
```shell
cd src
export CHROMIUM_BUILDTOOLS_PATH=`pwd`/buildtools
gn gen out/xxx
autoninja -C out/xxx radium
```

Synchronize chromium repository code every week.
