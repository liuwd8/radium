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
