# 02. Hello Triangle

## Intent
Draw the first visible triangle and observe how vertex positions and colors become fragments on the default framebuffer.

## Implementation Status
Implemented. This sample keeps the WGL setup from `01. WglBasic` and adds only the minimum draw path needed for a triangle.

## Prerequisite Sample
01. WglBasic

## New Concepts
- normalized device coordinates in the visible `[-1, 1]` range
- primitive type selection with `GL_TRIANGLES`
- per-vertex color assignment
- vertex submission order
- color interpolation across a triangle

## Compatibility Note
This sample intentionally uses `glBegin` / `glEnd` immediate mode. That is not the final modern OpenGL style, but it isolates the first-triangle concept before shader programs, vertex buffers, and vertex arrays are introduced in later samples.

## Expected Result
A 1280x720 window titled `02. Hello Triangle` opens, clears to a dark background, and renders one smoothly color-interpolated triangle.

## Important API Objects / Calls
- glViewport
- glClearColor
- glClear
- glBegin
- glColor3f
- glVertex2f
- glEnd
- SwapBuffers

## File Map
- `main.cpp`: owns the Win32 window, WGL context, frame loop, render call, swap, and cleanup.
- `LearningStage.h`: owns the clear color, small animation value, and triangle drawing code.
- `shaders/`: kept for repository consistency; this sample does not use shaders yet.
- `assets/`: kept for repository consistency; this sample does not use assets.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\02. Hello Triangle\02. Hello Triangle.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, clears the default framebuffer, draws a colored triangle, swaps buffers, and exits cleanly.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No shader, vertex buffer, texture, or extension loader is introduced yet.
