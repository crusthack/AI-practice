# Single Graphics Learning Project Template Instructions

Use this instruction when generating one learning sample project for DirectX 12, OpenGL, Vulkan, or another graphics API.

The goal is not to generate a full curriculum. The goal is to generate one self-contained project that can be repeated many times so every sample in a learning repository has the same code structure, file structure, and documentation contract.

## Required Inputs

Before generating a project, define:

```text
API_NAME: DirectX 12 | OpenGL | Vulkan | DirectX 11 | ...
PROJECT_NUMBER: two-digit sequence such as 01, 02, 03
PROJECT_NAME: short folder/display name such as HelloTriangle
LEARNING_GOAL: one sentence describing the single concept introduced by this sample
PREREQUISITE_SAMPLE: previous sample name or "none"
NEW_CONCEPTS: 3-6 concrete concepts introduced here
PRIMARY_API_CALLS: key API calls or objects that must appear in code
EXPECTED_VISUAL_RESULT: what the window should show when the sample runs
USER_INPUTS: keys/mouse controls, or "none"
ASSET_POLICY: inline/procedural assets, external assets copied to output, or generated assets
SHADER_POLICY: inline shader string, external shader source, precompiled shader binary, or API-specific equivalent
```

## Folder Layout

Create exactly this layout for one sample:

```text
{PROJECT_NUMBER}. {PROJECT_NAME}/
  {PROJECT_NUMBER}. {PROJECT_NAME}.vcxproj
  {PROJECT_NUMBER}. {PROJECT_NAME}.vcxproj.filters
  main.cpp
  LearningStage.h
  shaders/
    README.md
    {ProjectName}.shader_or_api_specific_extension
  assets/
    README.md
  README.md
```

Rules:

- `main.cpp` contains the common application flow and the minimum render loop.
- `LearningStage.h` contains only the concept-specific resources, update logic, and render hooks for this sample.
- `shaders/` exists even when shaders are inline. In that case, explain the policy in `shaders/README.md`.
- `assets/` exists even when assets are procedural. In that case, explain the policy in `assets/README.md`.
- The project must build independently from its `.vcxproj`.
- The project must also be easy to include in a root solution file.

## Reference Structure From This Repository

Use this DirectX 11 repository as the concrete reference for repeated sample structure.

Modernized reference sample:

```text
01. Dx11Basic/
  01. Dx11Basic.vcxproj
  01. Dx11Basic.vcxproj.filters
  main.cpp
  ClearTriangle.hlsl
```

Shared support files used by the modernized sample:

```text
common/
  D3DApp.h
  D3DApp.cpp
  DeviceResources.h
  DeviceResources.cpp
  ShaderUtils.h
  ShaderUtils.cpp
```

Legacy stage-extension pattern used by advanced samples:

```text
17. Model Loading/
  17. Model Loading.vcxproj
  17. Model Loading.vcxproj.filters
  main.cpp
  LearningStage.h
```

For new DX12/OpenGL/Vulkan repositories, prefer a hybrid of both:

- keep a small `common/` layer for window/device boilerplate;
- keep `main.cpp` readable and API-explicit;
- put sample-specific logic in `LearningStage.h`;
- keep shaders/assets in explicit folders or explicitly document why they are inline/procedural.

## Code Structure Contract

Every project must follow this high-level flow:

```text
wWinMain or main
  InitWindow
  InitGraphicsDevice
  CreateCommonResources
  ApplyStageSpecificSetup
  message loop
    UpdateFrameTime
    UpdateInput
    UpdateStageSpecificDemo
    Render
      BeginFrame
      RenderBaseSceneOrMinimalPrimitive
      ApplyStageSpecificRender
      EndFrame / Present
  ApplyStageSpecificCleanup
  CleanupCommonResources
```

For APIs with explicit frame graphs or command buffers, keep the same conceptual flow but map it to API-native names:

| Generic step | DirectX 12 | Vulkan | OpenGL |
|---|---|---|---|
| InitGraphicsDevice | device, command queue, swap chain | instance, physical/logical device, swapchain | context, loader, default framebuffer |
| BeginFrame | allocator/list reset, acquire back buffer | acquire image, begin command buffer | clear framebuffer |
| RenderBaseScene | command list draw | command buffer draw | draw calls |
| EndFrame | resource barrier, execute, present | end command buffer, submit, present | swap buffers |

## Required Files and Responsibilities

### `main.cpp`

Must contain:

- window creation and message loop;
- graphics device/swapchain/context initialization;
- frame timing;
- clear color and present;
- calls to all stage hooks:

```cpp
ApplyStageSpecificSetup(...);
UpdateStageSpecificDemo(timeSeconds);
ApplyStageSpecificRender(...);
ApplyStageSpecificCleanup();
```

Do not hide the API's first important objects too deeply. A learner should be able to find the key setup calls in `main.cpp`.

### `LearningStage.h`

Must contain:

- a top comment stating the sample's single learning goal;
- stage-specific structs, constants, and resource handles;
- `ApplyStageSpecificSetup`;
- `UpdateStageSpecificDemo`;
- `ApplyStageSpecificRender`;
- `ApplyStageSpecificCleanup`;
- short comments near the new concept only.

Do not put unrelated engine abstractions here. Do not introduce a second unrelated learning goal.

### Project `README.md`

Must contain:

```markdown
# {PROJECT_NUMBER}. {PROJECT_NAME}

## Intent
One sentence matching LEARNING_GOAL.

## New Concepts
- ...

## Expected Result
Describe exactly what appears on screen.

## Important API Objects / Calls
- ...

## File Map
- `main.cpp`: ...
- `LearningStage.h`: ...
- `shaders/`: ...
- `assets/`: ...

## Controls
List controls or say "none".

## Verification
Build command and short run expectation.

## Intent Match Checklist
- [ ] The project builds independently.
- [ ] The visual result demonstrates the stated intent.
- [ ] Every new API call listed above appears in code.
- [ ] No unrelated concept is introduced as a required dependency.
```

## Consistency Rules

- Each project introduces one primary concept.
- Reuse previous concepts only as scaffolding.
- Keep names literal: if the project is called `ImGui Integration`, it must either integrate real ImGui or clearly say `ImGui Integration Hook`.
- Keep the sample self-contained unless the purpose is explicitly asset loading.
- If an implementation is a stub or placeholder, state that in the project README and top-of-file comment.
- Prefer deterministic procedural assets for early samples.
- If external files are used, the project file must copy them to the output directory.
- Every project must have a smoke-testable visual result.

## API-Specific Adaptation Rules

### DirectX 12

- Show command queue, command allocator, command list, descriptor heaps, fences, swap chain, and resource barriers explicitly.
- `LearningStage.h` may own pipeline state objects, root signatures, descriptor tables, and sample-specific buffers.
- A project must not silently require a hidden engine layer to understand command submission.

### Vulkan

- Show instance, surface, physical device selection, logical device, queues, swapchain, render pass or dynamic rendering, framebuffers/images, command pool, command buffers, and synchronization explicitly.
- `LearningStage.h` may own pipeline layout, pipeline, descriptor sets, buffers, and sample-specific render commands.
- Always document which Vulkan extension/version feature is being used.

### OpenGL

- Show context creation, loader initialization, VAO/VBO setup, shader compilation, draw call, and buffer swap explicitly.
- `LearningStage.h` may own shader program, VAO/VBO, textures, and sample-specific uniform updates.
- Keep global GL state changes localized and restore or document them when relevant.

## Generation Procedure

When generating one sample:

1. Create the folder using `{PROJECT_NUMBER}. {PROJECT_NAME}`.
2. Create the required files from the folder layout.
3. Implement the minimum runnable window and graphics context.
4. Implement only the single `LEARNING_GOAL`.
5. Add comments only around the new API concept.
6. Add build file entries for every source, shader, and asset file.
7. Add a project README with the intent checklist.
8. Build the project.
9. Run a short smoke test and verify the expected visual result starts.
10. If the implementation differs from the intent, rename the project or update the intent before finishing.

## Copyable Generation Prompt

Use this prompt when asking an assistant or generator to create a single new graphics learning sample:

```text
Create one self-contained graphics learning sample project.

Do not create a full roadmap or multiple projects.

Use these inputs:
- API_NAME: <API>
- PROJECT_NUMBER: <NN>
- PROJECT_NAME: <Name>
- LEARNING_GOAL: <one primary concept>
- PREREQUISITE_SAMPLE: <previous sample or none>
- NEW_CONCEPTS: <3-6 concepts>
- PRIMARY_API_CALLS: <required API objects/calls>
- EXPECTED_VISUAL_RESULT: <what appears on screen>
- USER_INPUTS: <controls or none>
- ASSET_POLICY: <inline/procedural/external>
- SHADER_POLICY: <inline/external/precompiled/API-specific>

Create this folder structure:
{PROJECT_NUMBER}. {PROJECT_NAME}/
  {PROJECT_NUMBER}. {PROJECT_NAME}.vcxproj
  {PROJECT_NUMBER}. {PROJECT_NAME}.vcxproj.filters
  main.cpp
  LearningStage.h
  shaders/README.md
  assets/README.md
  README.md

Follow this code contract:
- main.cpp owns the window, graphics device/context/swapchain, frame loop, clear, present, and calls to stage hooks.
- LearningStage.h owns only the sample-specific resources and implements ApplyStageSpecificSetup, UpdateStageSpecificDemo, ApplyStageSpecificRender, and ApplyStageSpecificCleanup.
- The project README must state intent, expected result, controls, important API calls, file map, verification command, and an intent-match checklist.
- The sample must build independently and have a smoke-testable visual result.
- If any implementation is a placeholder, label it as a placeholder in both code comments and README.

After generating, build the project and verify that the visual result matches LEARNING_GOAL.
```

## Example Filled Context

```text
API_NAME: Vulkan
PROJECT_NUMBER: 03
PROJECT_NAME: VertexBuffer
LEARNING_GOAL: Upload triangle vertices into a GPU buffer and draw from that buffer.
PREREQUISITE_SAMPLE: 02. HelloTriangle
NEW_CONCEPTS: vertex buffer, host-visible staging data, vertex input binding, vertex attribute descriptions
PRIMARY_API_CALLS: vkCreateBuffer, vkAllocateMemory, vkBindBufferMemory, vkMapMemory, vkCmdBindVertexBuffers, vkCmdDraw
EXPECTED_VISUAL_RESULT: a colored triangle rendered from vertex buffer data
USER_INPUTS: none
ASSET_POLICY: no external assets
SHADER_POLICY: external GLSL compiled to SPIR-V or precompiled SPIR-V copied to output
```

Generated project must still follow the same folder layout and hook names even if the API implementation details differ.
