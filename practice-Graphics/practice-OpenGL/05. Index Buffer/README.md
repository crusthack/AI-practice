# 05. Index Buffer

## Intent
Reuse vertices through an element/index buffer and draw indexed triangles with `glDrawElements`.

## Implementation Status
Implemented.

## Prerequisite Sample
04. Vertex Buffer

## New Concepts
- `GL_ELEMENT_ARRAY_BUFFER` stores integer indices that reference vertices in the bound vertex buffer.
- A quad is represented as four unique vertices and six indices, so the two triangles share the diagonal vertices instead of duplicating them.
- `glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr)` replaces `glDrawArrays`.
- The vertex buffer, index buffer, shader program, and draw call are bound explicitly in the render hook.
- VAO state capture is still deferred to `06. Vertex Array Object`.

## Expected Result
A 1280x720 window titled `05. Index Buffer` opens, clears to a dark blue-green background, and renders a color-interpolated quad made from two indexed triangles.

## Important API Objects / Calls
- `wglGetProcAddress`
- `glCreateShader`
- `glCreateProgram`
- `glGenBuffers`
- `glBindBuffer`
- `glBufferData`
- `glVertexPointer`
- `glColorPointer`
- `glDrawElements`
- `glDeleteBuffers`

## File Map
- `main.cpp`: owns the Win32 window, WGL context, frame loop, swap, and stage hook calls.
- `LearningStage.h`: owns shader setup, VBO/EBO creation, indexed draw, and cleanup.
- `shaders/`: kept for the DirectX-style repository contract; this early sample embeds tiny shader strings to keep the lesson self-contained.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\05. Index Buffer\05. Index Buffer.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\OpenGLLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, uploads vertex and index data, draws the quad with `glDrawElements`, swaps buffers, and exits cleanly.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Vertex reuse is represented by an element/index buffer.
- [x] Indexed drawing replaces non-indexed drawing.
- [x] VAO concepts are left for the next sample.
