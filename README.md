# Radium

This project is designed to solve cross-platform scenarios that require webview. Chromium Views + Chromium Content provide a complete set of capabilities. Mainly for large-scale application of WebUI.

Official explanation about WebUI: https://chromium.googlesource.com/chromium/src/+/main/docs/webui_explainer.md

This project is based on the Chromium open source project. Use //content + //ui and other required components.

# Supported Platforms

Planned to support Win/Mac/Linux/Android. Currently only Linux/Win/Mac platforms are supported.

# Demo

## Linux Demo
[Linux Demo](https://github.com/liuwd8/chromium_application/blob/main/demo.mp4)

## Mac Demo
[Mac Demo](https://github.com/user-attachments/assets/85761526-2f84-4d03-b59c-ed0528e30c26)

## Win Demo (with dark mode)
[Win Demo](https://github.com/user-attachments/assets/c6e4b644-0c83-4357-9549-af80179efe57)

[Win Demo(dark mode)](https://github.com/user-attachments/assets/1f734f61-6508-4359-92a8-fbd9b1de216c)

# Build
To compile this project, you need to pull the Chromium project first. For the documentation of the Chromium project, please refer to: https://www.chromium.org/developers/how-tos/get-the-code/

After pulling Chromium, apply the patch file under patches to the chromium project:
```
git am --reject patches/*.patch
```

If there is a .reject after the patch, you need to manually resolve the conflict

Add to the .gclient file

```
    "custom_vars": {
      "checkout_radium": True,
    },
```

After that you need to re-run `gclient sync`. Then you can start compiling
```shell
autoninja -C out/xxx radium
```

Currently based on chromium main branch (commit: 4a21fad763705b6a4996c5cf07758f8cd185c78d. Each rebase will update the commit value). Synchronize chromium repository code every week.
