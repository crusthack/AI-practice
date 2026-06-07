# 03. Shader Program

## Intent
Compile GLSL vertex and fragment shaders, link a program, validate errors, bind the program, and render the triangle through that program.

## Implementation Status
Implemented.

## Prerequisite Sample
02. Hello Triangle

## New Concepts
- OpenGL 1.1 headers on Windows do not expose shader entry points, so this sample loads the required functions with `wglGetProcAddress` after the context exists.
- A shader object is created with `glCreateShader`, populated with `glShaderSource`, compiled with `glCompileShader`, and checked through `glGetShaderiv`.
- A program object is created with `glCreateProgram`, receives attached shader objects, links with `glLinkProgram`, and is checked through `glGetProgramiv`.
- Shader compiler and linker logs are queried through `GL_INFO_LOG_LENGTH` and reported as exceptions.
- The sample intentionally uses GLSL 1.20 compatibility built-ins (`gl_Vertex`, `gl_Color`, `gl_FragColor`) so VBO/VAO setup remains reserved for the following lessons.

## Expected Result
A 1280x720 window titled `03. Shader Program` opens, clears to a dark blue-green background, and renders a color-interpolated triangle through a GLSL shader program.

## Important API Objects / Calls
- `wglGetProcAddress`
- `glCreateShader`
- `glShaderSource`
- `glCompileShader`
- `glGetShaderiv`
- `glGetShaderInfoLog`
- `glCreateProgram`
- `glAttachShader`
- `glLinkProgram`
- `glGetProgramiv`
- `glGetProgramInfoLog`
- `glUseProgram`
- `glDeleteShader`
- `glDeleteProgram`

## File Map
- `main.cpp`: owns the Win32 window, WGL context, frame loop, swap, and stage hook calls.
- `LearningStage.h`: owns shader API loading, shader/program creation, compile/link error handling, rendering, and cleanup.
- `shaders/`: kept for the DirectX-style repository contract; this early sample embeds tiny shader strings to keep the lesson self-contained.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\03. Shader Program\03. Shader Program.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\OpenGLLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, compiles and links the GLSL program, draws the triangle through `glUseProgram`, swaps buffers, and exits cleanly.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Shader compile and program link errors are surfaced with info logs.
- [x] Buffer object and VAO concepts are left for later samples.
