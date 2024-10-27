# Radium

这个项目是为了解决需要 webview 的跨平台场景。 Chromium Views + Chromium Content 提供整套能力。主要为了 WebUI 的大规模应用。

本项目基于 Chromium 开源项目。 使用 //content + //ui 和其他需要的组件。

目前的 electron 在架构上太过繁重，且无法开启沙箱。 基于 Views + WebUI 我们可以完全打造支持各平台的应用。

计划支持 Win/Mac/Linux/Android. 目前仅支持了 Linux 平台.

# 演示

[演示视频（demo）](https://github.com/user-attachments/assets/0e8821fd-baff-4c87-ad90-b4f5966705ff)

# 编译
编译该项目需要先拉取 Chromium 项目。 Chromium 项目的文档请参考: https://www.chromium.org/developers/how-tos/get-the-code/

在拉取 Chromium 项目的基础上。拉取完成 Chromium 后, 应用 patches 下的 patch 文件到chromium 项目:

```
git am --reject patches/*.patch
```

如果patch 之后存在.reject 需要手动解决冲突

在 .gclient 文件中加入

```
    "custom_vars": {
      "checkout_radium": True,
    },
```

在这之后需要重新 `gclient sync`. 之后可以开始编译
```shell
autoninja -C out/xxx radium
```

目前基于 chromium main branch。每周同步 chromium 仓库代码。
