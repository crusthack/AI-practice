# 01. WglBasic

## Intent
Create the smallest useful Windows OpenGL program: a Win32 window, a WGL rendering context, a default framebuffer clear, and a buffer swap.

## Implementation Status
Implemented. This sample intentionally uses only Win32, WGL, and OpenGL 1.1 entry points available through `opengl32.lib`.

## Prerequisite Sample
none

## New Concepts
- Win32 window class registration and `CS_OWNDC`
- device context (`HDC`) acquisition with `GetDC`
- pixel format selection with `ChoosePixelFormat` / `SetPixelFormat`
- WGL context creation with `wglCreateContext`
- binding the context with `wglMakeCurrent`
- clearing the default framebuffer with `glClearColor` / `glClear`
- presenting with `SwapBuffers`

## Expected Result
A 1280x720 window titled `01. WglBasic` opens and shows a solid dark blue-gray background.

## Important API Objects / Calls
- RegisterClassExW
- CreateWindowExW
- GetDC
- ChoosePixelFormat
- SetPixelFormat
- wglCreateContext
- wglMakeCurrent
- glViewport
- glClearColor
- glClear
- SwapBuffers
- wglDeleteContext
- ReleaseDC

## File Map
- `main.cpp`: owns Win32 window creation, WGL context setup, message loop, render call, buffer swap, and shutdown.
- `LearningStage.h`: owns the stage-specific clear color and default-framebuffer render hook.
- `shaders/`: kept for repository consistency; this sample does not use shaders.
- `assets/`: kept for repository consistency; this sample does not use assets.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\01. WglBasic\01. WglBasic.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, clears the default framebuffer every frame, swaps buffers, and exits cleanly.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No shader, vertex buffer, texture, or modern extension loader is introduced yet.
