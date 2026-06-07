# 11. Depth Buffer

## Intent
Enable depth testing so nearer fragments occlude farther fragments.

## Implementation Status
Scaffold. This sample is reserved for its numbered OpenGL lesson and should be implemented after the previous sample is complete.

## Prerequisite Sample
10. Texture Sampling

## New Concepts
- OpenGL state changes are explicit and scoped by the stage render hook.
- Modern OpenGL resources are created through VAO/VBO/EBO, shader programs, textures, FBOs, UBOs, and instancing where the stage requires them.
- The Win32/WGL application shell is shared, while each sample keeps a numbered `LearningStage.h` entry point.
- Shader and asset folders stay present for the DirectX-style repository contract.

## Expected Result
A 1280x720 window titled `11. Depth Buffer` opens, clears to a dark blue-green background, and continues presenting until closed.

## Important API Objects / Calls
- RegisterClassExW
- CreateWindowExW
- GetDC
- ChoosePixelFormat
- SetPixelFormat
- wglCreateContext
- wglMakeCurrent
- wglCreateContextAttribsARB
- wglGetProcAddress
- glCreateShader / glCreateProgram
- glGenVertexArrays / glBindVertexArray
- glGenBuffers / glBindBuffer
- glVertexAttribPointer
- glDrawArrays / glDrawElements
- SwapBuffers

## File Map
- `main.cpp`: owns the Win32 window, WGL context, frame loop, swap, and stage hook calls.
- `LearningStage.h`: owns sample-specific OpenGL state, resources, update logic, render logic, and cleanup.
- `shaders/`: contains `Depth Buffer.glsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\11. Depth Buffer\11. Depth Buffer.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\OpenGLLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, clears the default framebuffer, swaps buffers, and exits cleanly.

## Intent Match Checklist
- [ ] The project builds independently.
- [ ] The visual result demonstrates the stated intent.
- [ ] Every new API call listed above appears in code.
- [ ] No unrelated concept is introduced as a required dependency.
