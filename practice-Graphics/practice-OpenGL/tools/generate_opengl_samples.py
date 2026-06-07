from __future__ import annotations

import hashlib
import textwrap
import uuid
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


SAMPLES = [
    ("01", "WglBasic", "Create a Win32 window, initialize a WGL OpenGL context, clear the default framebuffer, and swap buffers.", "none", "Implemented"),
    ("02", "Hello Triangle", "Draw the first triangle and identify how OpenGL turns submitted vertices into fragments.", "01. WglBasic", "Implemented"),
    ("03", "Shader Program", "Compile GLSL vertex and fragment shaders, link a program, and inspect shader errors.", "02. Hello Triangle", "Implemented"),
    ("04", "Vertex Buffer", "Upload vertex data into a VBO and describe the vertex attribute layout.", "03. Shader Program", "Implemented"),
    ("05", "Index Buffer", "Reuse vertices through an element buffer and indexed drawing.", "04. Vertex Buffer", "Implemented"),
    ("06", "Vertex Array Object", "Capture vertex input state with a VAO and separate object setup from draw calls.", "05. Index Buffer", "Scaffold"),
    ("07", "Uniforms And Time", "Update shader uniforms every frame for animation and per-draw parameters.", "06. Vertex Array Object", "Scaffold"),
    ("08", "Transform Matrices", "Send model, view, and projection matrices to GLSL and transform geometry.", "07. Uniforms And Time", "Scaffold"),
    ("09", "Camera", "Build an orbit camera and update the view matrix from keyboard input.", "08. Transform Matrices", "Scaffold"),
    ("10", "Texture Sampling", "Create a procedural texture, configure sampler state, and sample it in GLSL.", "09. Camera", "Scaffold"),
    ("11", "Depth Buffer", "Enable depth testing so nearer fragments occlude farther fragments.", "10. Texture Sampling", "Scaffold"),
    ("12", "Face Culling", "Control front-face winding and cull back or front faces.", "11. Depth Buffer", "Scaffold"),
    ("13", "Blending", "Enable alpha blending and render transparent geometry in controlled order.", "12. Face Culling", "Scaffold"),
    ("14", "Framebuffer Object", "Render into an offscreen framebuffer object and sample its color attachment.", "13. Blending", "Scaffold"),
    ("15", "Post Processing", "Draw a fullscreen pass from an FBO texture and apply a simple effect.", "14. Framebuffer Object", "Scaffold"),
    ("16", "Uniform Buffer Object", "Group frame constants in a UBO and bind it across shader programs.", "15. Post Processing", "Scaffold"),
    ("17", "Texture Array", "Store related textures in a texture array and choose layers in the shader.", "16. Uniform Buffer Object", "Scaffold"),
    ("18", "Cubemap Skybox", "Create a cubemap texture and render a skybox with depth-state changes.", "17. Texture Array", "Scaffold"),
    ("19", "Model Loading", "Load or procedurally parse mesh data and upload it into GL buffers.", "18. Cubemap Skybox", "Scaffold"),
    ("20", "Scene Graph", "Render multiple objects with transforms, mesh references, and material data.", "19. Model Loading", "Scaffold"),
    ("21", "Lighting And Materials", "Combine normals, material parameters, and lights for Blinn-Phong shading.", "20. Scene Graph", "Scaffold"),
    ("22", "Instancing", "Draw many copies of a mesh with per-instance transform or color data.", "21. Lighting And Materials", "Scaffold"),
    ("23", "Shadow Mapping", "Render depth from a light view and sample it when shading the main scene.", "22. Instancing", "Scaffold"),
    ("24", "Geometry Shader", "Use a geometry shader for controlled primitive expansion or visualization.", "23. Shadow Mapping", "Scaffold"),
    ("25", "Compute Shader", "Dispatch a compute shader and synchronize its result for rendering or readback.", "24. Geometry Shader", "Scaffold"),
    ("26", "ImGui Integration", "Attach Dear ImGui as a debug UI overlay for OpenGL runtime parameters.", "25. Compute Shader", "Scaffold"),
    ("27", "Mini Renderer", "Combine context setup, shader management, scene data, render passes, and debug UI into a small renderer.", "26. ImGui Integration", "Scaffold"),
]


def stable_guid(text: str) -> str:
    digest = hashlib.sha1(text.encode("utf-8")).hexdigest()
    return "{" + str(uuid.UUID(digest[:32])).upper() + "}"


def namespace_name(sample_name: str) -> str:
    return "".join(ch for ch in sample_name if ch.isalnum())


def shader_file_name(sample_name: str) -> str:
    return f"{sample_name}.glsl"


def write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")


def make_vcxproj(no: str, name: str, guid: str) -> str:
    ns = namespace_name(name)
    shader = shader_file_name(name)
    return f"""<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <Keyword>Win32Proj</Keyword>
    <ProjectGuid>{guid}</ProjectGuid>
    <RootNamespace>{ns}</RootNamespace>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v145</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v145</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings" />
  <ImportGroup Label="Shared" />
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup>
    <OutDir>$(ProjectDir)bin\\$(Platform)\\$(Configuration)\\</OutDir>
    <IntDir>$(ProjectDir)obj\\$(Platform)\\$(Configuration)\\</IntDir>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <WarningLevel>Level4</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;_DEBUG;UNICODE;_UNICODE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp17</LanguageStandard>
    </ClCompile>
    <Link>
      <SubSystem>Windows</SubSystem>
      <AdditionalDependencies>opengl32.lib;gdi32.lib;user32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level4</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;NDEBUG;UNICODE;_UNICODE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp17</LanguageStandard>
    </ClCompile>
    <Link>
      <SubSystem>Windows</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <AdditionalDependencies>opengl32.lib;gdi32.lib;user32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="main.cpp" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="LearningStage.h" />
    <ClInclude Include="..\\common\\LearningStages.h" />
    <ClInclude Include="..\\common\\OpenGLApp.h" />
    <ClInclude Include="..\\common\\OpenGLRuntime.h" />
  </ItemGroup>
  <ItemGroup>
    <None Include="README.md" />
    <None Include="assets\\README.md" />
    <None Include="shaders\\README.md" />
    <None Include="shaders\\{shader}" />
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets" />
</Project>
"""


def make_filters(name: str) -> str:
    shader = shader_file_name(name)
    return f"""<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="Source Files">
      <UniqueIdentifier>{{0f08dc8d-efb2-4c3e-bca5-773630652efe}}</UniqueIdentifier>
      <Extensions>cpp;c;cc;cxx</Extensions>
    </Filter>
    <Filter Include="Header Files">
      <UniqueIdentifier>{{602ff439-cbbf-4fb0-8eaf-d3093f9235e3}}</UniqueIdentifier>
      <Extensions>h;hpp;hxx</Extensions>
    </Filter>
    <Filter Include="Shaders">
      <UniqueIdentifier>{{d74b7d8f-d40a-40b2-8318-24a149954ca0}}</UniqueIdentifier>
    </Filter>
    <Filter Include="Assets">
      <UniqueIdentifier>{{9407f8a2-4102-4e52-b594-8a6b8473ee99}}</UniqueIdentifier>
    </Filter>
    <Filter Include="Common">
      <UniqueIdentifier>{{6fdc2c3c-7d0e-4b3e-8cf2-5942f31e918d}}</UniqueIdentifier>
    </Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="main.cpp">
      <Filter>Source Files</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="LearningStage.h">
      <Filter>Header Files</Filter>
    </ClInclude>
    <ClInclude Include="..\\common\\LearningStages.h">
      <Filter>Common</Filter>
    </ClInclude>
    <ClInclude Include="..\\common\\OpenGLApp.h">
      <Filter>Common</Filter>
    </ClInclude>
    <ClInclude Include="..\\common\\OpenGLRuntime.h">
      <Filter>Common</Filter>
    </ClInclude>
  </ItemGroup>
  <ItemGroup>
    <None Include="README.md" />
    <None Include="shaders\\README.md">
      <Filter>Shaders</Filter>
    </None>
    <None Include="shaders\\{shader}">
      <Filter>Shaders</Filter>
    </None>
    <None Include="assets\\README.md">
      <Filter>Assets</Filter>
    </None>
  </ItemGroup>
</Project>
"""


def make_main(no: str, name: str) -> str:
    title = f"{no}. {name}"
    cls = f"OpenGL{no}{namespace_name(name)}WindowClass"
    return f"""#define OPENGL_SAMPLE_NUMBER {int(no)}
#define OPENGL_SAMPLE_TITLE L"{title}"
#define OPENGL_SAMPLE_CLASS L"{cls}"

#include "LearningStage.h"
#include "../common/OpenGLApp.h"
"""


def make_stage(goal: str, status: str) -> str:
    return f"""#pragma once

// Learning goal: {goal}
// Implementation status: {status}.

#include "../common/LearningStages.h"
"""


def make_readme(no: str, name: str, goal: str, prereq: str, status: str) -> str:
    title = f"{no}. {name}"
    checkbox = "x" if status == "Implemented" else " "
    return f"""# {title}

## Intent
{goal}

## Implementation Status
{status}. This project is wired to the shared Win32 + WGL + OpenGL core-profile runtime and enables the stage-specific rendering path for this lesson.

## Prerequisite Sample
{prereq}

## New Concepts
- OpenGL state changes are explicit and scoped by the stage render hook.
- Modern OpenGL resources are created through VAO/VBO/EBO, shader programs, textures, FBOs, UBOs, and instancing where the stage requires them.
- The Win32/WGL application shell is shared, while each sample keeps a numbered `LearningStage.h` entry point.
- Shader and asset folders stay present for the DirectX-style repository contract.

## Expected Result
A 1280x720 window titled `{title}` opens, clears to a dark blue-green background, and continues presenting until closed.

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
- `shaders/`: contains `{shader_file_name(name)}` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe" ".\\{title}\\{title}.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe" ".\\OpenGLLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, clears the default framebuffer, swaps buffers, and exits cleanly.

## Intent Match Checklist
- [{checkbox}] The project builds independently.
- [{checkbox}] The visual result demonstrates the stated intent.
- [{checkbox}] Every new API call listed above appears in code.
- [{checkbox}] No unrelated concept is introduced as a required dependency.
"""


def make_shader_readme(status: str) -> str:
    return f"""# Shaders

Policy: GLSL shader sources for this sample live in this folder.

Current status: {status}. The scaffold keeps a placeholder `.glsl` file so the Visual Studio project and repository layout stay consistent. Once a sample uses programmable pipeline features, keep vertex, fragment, geometry, or compute shader source here unless the README states a different policy.
"""


def make_asset_readme() -> str:
    return """# Assets

Policy: no required external assets by default.

Prefer procedural data for early samples. When a later sample introduces textures, models, or UI resources, document the source, runtime copy expectations, and fallback behavior here.
"""


def make_shader_placeholder(no: str, name: str, goal: str) -> str:
    return f"""// {no}. {name}
// Learning goal: {goal}
// Placeholder GLSL file. Add stage-specific shaders here when this sample is promoted.
"""


def make_context() -> str:
    implemented = sum(1 for *_rest, status in SAMPLES if status == "Implemented")
    scaffold = len(SAMPLES) - implemented
    lines = "\n".join(f"- `{no}. {name}`: {status}" for no, name, _goal, _prereq, status in SAMPLES)
    return f"""# OpenGL Practice Workspace Context

## Purpose

This workspace is an OpenGL learning repository built around small, repeatable Visual C++ sample projects. It mirrors the DirectX 11 and DirectX 12 practice repositories by keeping one concept per sample and a consistent file/documentation contract.

## Current State

- Root solution: `OpenGLLearning.sln`
- Current API focus: OpenGL through Win32 WGL
- Current platform: Windows, x64
- Current Visual Studio toolset: `v145`
- Project count: {len(SAMPLES)}
- Implemented sample count: {implemented}
- Scaffolded sample count: {scaffold}

## Samples

{lines}

## Repository Layout Contract

```text
NN. SampleName/
  NN. SampleName.vcxproj
  NN. SampleName.vcxproj.filters
  main.cpp
  LearningStage.h
  shaders/
    README.md
    SampleName.glsl
  assets/
    README.md
  README.md
```

## Sample Contract

`main.cpp` owns the application shell:

- Win32 window creation
- WGL pixel format and OpenGL context setup
- frame timing and message loop
- buffer swap and shutdown
- calls to the stage hooks

`LearningStage.h` owns only sample-specific code:

- stage resources
- setup/update/render/cleanup hooks
- comments around the one new concept introduced by the sample

## Build Notes

Build the whole workspace:

```powershell
& "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe" ".\\OpenGLLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Generated build outputs are placed under each sample's `bin/` and `obj/` directories and should stay out of source control.

## Expansion Rules

- Add each new sample project to `OpenGLLearning.sln`.
- Keep each sample independently buildable from its `.vcxproj`.
- Keep one primary learning goal per sample.
- Prefer procedural data for early samples.
- Keep OpenGL extension loading explicit through `common/OpenGLRuntime.h`.
- Keep stage-specific behavior controlled by the sample number and documented in each README.
"""


def make_roadmap() -> str:
    rows = "\n".join(f"| {no} | `{name}` | {goal} | {status} |" for no, name, goal, _prereq, status in SAMPLES)
    return f"""# OpenGL Learning Roadmap

This roadmap keeps the 27-step learning scale of the DirectX 11 and DirectX 12 practice sets, but the sequence is organized around OpenGL's context, global state, buffer objects, shader programs, textures, framebuffers, and render-pass model.

## Learning Principles

- One sample, one primary concept.
- Keep the WGL application shell explicit in `main.cpp`.
- Put only sample-specific OpenGL resources and hooks in `LearningStage.h`.
- Prefer deterministic procedural data before asset-loading samples.
- Every sample must have a visible result that can be checked quickly.
- Every sample must build independently and also belong to `OpenGLLearning.sln`.

## Full Sequence

| No. | Sample | Goal | Status |
|---|---|---|---|
{rows}

## Implementation Strategy

All projects are implemented as independent OpenGL projects with the required folder contract, README contract, shader/assets folders, and visible stage-specific behavior. The shared runtime keeps WGL setup, OpenGL function loading, shader compilation, and reusable render helpers in `common/`.

## Next Implementation Pass

Future passes should deepen individual lessons while preserving the current ordering and build contract:

- keep one main concept per numbered sample
- prefer procedural data unless the lesson is explicitly about external assets
- update the sample README when a stage introduces a new OpenGL object or state transition
"""


def make_readme_root() -> str:
    rows = "\n".join(f"| {no} | {name} | {goal} | {status} |" for no, name, goal, _prereq, status in SAMPLES)
    return f"""# OpenGL Learning Samples

OpenGL을 단계적으로 익히기 위한 Visual Studio C++ 샘플 모음이다. DirectX11/12 학습 프로젝트와 같은 27단계 구조를 유지하되, WGL 컨텍스트 생성부터 버퍼, 셰이더, 텍스처, FBO, 고급 렌더링까지 OpenGL식 개념 순서로 정리한다.

## Quick Start

Visual Studio에서 `OpenGLLearning.sln`을 열고 원하는 번호의 프로젝트를 시작 프로젝트로 설정한 뒤 `F5`로 실행한다.

Command-line build:

```powershell
& "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe" ".\\OpenGLLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

## Structure

```text
practice-OpenGL/
  OpenGLLearning.sln
  WORKSPACE_CONTEXT.md
  ROADMAP.md
  docs/
  tools/
  01. WglBasic/
    main.cpp
    LearningStage.h
    shaders/
    assets/
  ...
  27. Mini Renderer/
```

Each project is independently buildable and uses the same hook pattern:

```text
wWinMain
  InitWindow
  InitOpenGL
  ApplyStageSpecificSetup
  message loop
    UpdateStageSpecificDemo
    ApplyStageSpecificRender
    SwapBuffers
  ApplyStageSpecificCleanup
```

## Curriculum

| # | Project | Core Goal | Status |
|---|---|---|---|
{rows}

## Notes

- The project intentionally avoids external dependencies such as GLFW, GLAD, or GLEW.
- Samples use Win32 + WGL and request an OpenGL core profile context.
- Modern OpenGL entry points are loaded explicitly through `wglGetProcAddress` in `common/OpenGLRuntime.h`.
"""


def make_docs() -> dict[str, str]:
    return {
        "01-09-opengl-foundation-guide.md": """# OpenGL Learning Guide - 01~09

This phase covers the minimum Windows OpenGL runtime, first draw calls, shader programs, vertex input, matrix transforms, and camera movement.

Key comparison with the DirectX samples:

- WGL context creation replaces D3D device/swap-chain setup.
- OpenGL state is context-global, so samples should be careful about what state each stage changes.
- Buffer and shader ownership still belongs in `LearningStage.h`, matching the DirectX12 stage-hook contract.
""",
        "10-18-texture-state-framebuffer-guide.md": """# OpenGL Learning Guide - 10~18

This phase introduces texture sampling, depth/cull/blend state, framebuffer objects, post processing, UBOs, texture arrays, and cubemaps.

State tracking is the main learning risk. Each sample should document the exact state it enables, disables, binds, or restores.
""",
        "19-27-advanced-rendering-tooling-guide.md": """# OpenGL Learning Guide - 19~27

This phase introduces model data, scene structure, lighting, instancing, shadow mapping, geometry/compute shaders, ImGui, and the final mini renderer.

The final projects may add extension loading and optional third-party integration, but those dependencies should be documented in the sample README before they become required.
""",
    }


def make_gitignore() -> str:
    return """bin/
obj/
*.user
*.suo
*.VC.db
*.VC.VC.opendb
.vs/
"""


def make_sln(projects: list[tuple[str, str, str]]) -> str:
    project_type = "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"
    lines = [
        "Microsoft Visual Studio Solution File, Format Version 12.00",
        "# Visual Studio Version 18",
        "VisualStudioVersion = 18.0.36231.0",
        "MinimumVisualStudioVersion = 10.0.40219.1",
    ]
    for no, name, guid in projects:
        title = f"{no}. {name}"
        lines.append(f'Project("{project_type}") = "{title}", "{title}\\{title}.vcxproj", "{guid}"')
        lines.append("EndProject")
    lines.extend([
        "Global",
        "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution",
        "\t\tDebug|x64 = Debug|x64",
        "\t\tRelease|x64 = Release|x64",
        "\tEndGlobalSection",
        "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution",
    ])
    for _no, _name, guid in projects:
        lines.extend([
            f"\t\t{guid}.Debug|x64.ActiveCfg = Debug|x64",
            f"\t\t{guid}.Debug|x64.Build.0 = Debug|x64",
            f"\t\t{guid}.Release|x64.ActiveCfg = Release|x64",
            f"\t\t{guid}.Release|x64.Build.0 = Release|x64",
        ])
    lines.extend([
        "\tEndGlobalSection",
        "\tGlobalSection(SolutionProperties) = preSolution",
        "\t\tHideSolutionNode = FALSE",
        "\tEndGlobalSection",
        "EndGlobal",
        "",
    ])
    return "\n".join(lines)


def main() -> None:
    projects: list[tuple[str, str, str]] = []
    for no, name, goal, prereq, status in SAMPLES:
        title = f"{no}. {name}"
        guid = stable_guid(f"practice-OpenGL/{title}")
        projects.append((no, name, guid))
        sample_dir = ROOT / title
        if no not in {"01", "02", "03", "04", "05"} or not sample_dir.exists():
            write(sample_dir / f"{title}.vcxproj", make_vcxproj(no, name, guid))
            write(sample_dir / f"{title}.vcxproj.filters", make_filters(name))
            write(sample_dir / "main.cpp", make_main(no, name))
            write(sample_dir / "LearningStage.h", make_stage(goal, status))
            write(sample_dir / "README.md", make_readme(no, name, goal, prereq, status))
            write(sample_dir / "shaders" / "README.md", make_shader_readme(status))
            write(sample_dir / "shaders" / shader_file_name(name), make_shader_placeholder(no, name, goal))
            write(sample_dir / "assets" / "README.md", make_asset_readme())

    write(ROOT / "OpenGLLearning.sln", make_sln(projects))
    write(ROOT / ".gitignore", make_gitignore())
    write(ROOT / "WORKSPACE_CONTEXT.md", make_context())
    write(ROOT / "ROADMAP.md", make_roadmap())
    write(ROOT / "README.md", make_readme_root())
    for filename, content in make_docs().items():
        write(ROOT / "docs" / filename, content)


if __name__ == "__main__":
    main()
