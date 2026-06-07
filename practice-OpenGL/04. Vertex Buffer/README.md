# 04. Vertex Buffer

## Intent
Upload vertex data into an OpenGL vertex buffer object, describe the interleaved position/color layout, and draw the triangle with `glDrawArrays`.

## Implementation Status
Implemented.

## Prerequisite Sample
03. Shader Program

## New Concepts
- Buffer object entry points are loaded explicitly with `wglGetProcAddress`.
- A VBO is created with `glGenBuffers`, selected with `glBindBuffer`, initialized with `glBufferData`, and released with `glDeleteBuffers`.
- Vertex data is stored as an interleaved C++ `Vertex` structure: two floats for clip-space position followed by three floats for color.
- The sample binds the VBO and uses `glVertexPointer` / `glColorPointer` to describe byte offsets and stride for compatibility-profile vertex arrays.
- `glDrawArrays(GL_TRIANGLES, 0, vertexCount)` replaces `glBegin` / `glEnd` from the previous samples.
- VAO state capture is intentionally not introduced yet; that remains the goal of `06. Vertex Array Object`.

## Expected Result
A 1280x720 window titled `04. Vertex Buffer` opens, clears to a dark blue-green background, and renders the color-interpolated triangle from data stored in a VBO.

## Important API Objects / Calls
- `wglGetProcAddress`
- `glCreateShader`
- `glCreateProgram`
- `glUseProgram`
- `glGenBuffers`
- `glBindBuffer`
- `glBufferData`
- `glEnableClientState`
- `glVertexPointer`
- `glColorPointer`
- `glDrawArrays`
- `glDeleteBuffers`

## File Map
- `main.cpp`: owns the Win32 window, WGL context, frame loop, swap, and stage hook calls.
- `LearningStage.h`: owns shader setup from 03 plus VBO creation, vertex layout description, draw, and cleanup.
- `shaders/`: kept for the DirectX-style repository contract; this early sample embeds tiny shader strings to keep the lesson self-contained.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\04. Vertex Buffer\04. Vertex Buffer.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\OpenGLLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, uploads the triangle data to a VBO, draws it with `glDrawArrays`, swaps buffers, and exits cleanly.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Vertex data is uploaded through a VBO.
- [x] Immediate-mode drawing is removed from this sample.
- [x] VAO and index-buffer concepts are left for later samples.
