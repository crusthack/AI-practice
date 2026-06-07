# OpenGL Learning Roadmap

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
| 01 | `WglBasic` | Create a Win32 window, initialize a WGL OpenGL context, clear the default framebuffer, and swap buffers. | Implemented |
| 02 | `Hello Triangle` | Draw the first triangle and identify how OpenGL turns submitted vertices into fragments. | Implemented |
| 03 | `Shader Program` | Compile GLSL vertex and fragment shaders, link a program, and inspect shader errors. | Implemented |
| 04 | `Vertex Buffer` | Upload vertex data into a VBO and describe the vertex attribute layout. | Implemented |
| 05 | `Index Buffer` | Reuse vertices through an element buffer and indexed drawing. | Implemented |
| 06 | `Vertex Array Object` | Capture vertex input state with a VAO and separate object setup from draw calls. | Scaffold |
| 07 | `Uniforms And Time` | Update shader uniforms every frame for animation and per-draw parameters. | Scaffold |
| 08 | `Transform Matrices` | Send model, view, and projection matrices to GLSL and transform geometry. | Scaffold |
| 09 | `Camera` | Build an orbit camera and update the view matrix from keyboard input. | Scaffold |
| 10 | `Texture Sampling` | Create a procedural texture, configure sampler state, and sample it in GLSL. | Scaffold |
| 11 | `Depth Buffer` | Enable depth testing so nearer fragments occlude farther fragments. | Scaffold |
| 12 | `Face Culling` | Control front-face winding and cull back or front faces. | Scaffold |
| 13 | `Blending` | Enable alpha blending and render transparent geometry in controlled order. | Scaffold |
| 14 | `Framebuffer Object` | Render into an offscreen framebuffer object and sample its color attachment. | Scaffold |
| 15 | `Post Processing` | Draw a fullscreen pass from an FBO texture and apply a simple effect. | Scaffold |
| 16 | `Uniform Buffer Object` | Group frame constants in a UBO and bind it across shader programs. | Scaffold |
| 17 | `Texture Array` | Store related textures in a texture array and choose layers in the shader. | Scaffold |
| 18 | `Cubemap Skybox` | Create a cubemap texture and render a skybox with depth-state changes. | Scaffold |
| 19 | `Model Loading` | Load or procedurally parse mesh data and upload it into GL buffers. | Scaffold |
| 20 | `Scene Graph` | Render multiple objects with transforms, mesh references, and material data. | Scaffold |
| 21 | `Lighting And Materials` | Combine normals, material parameters, and lights for Blinn-Phong shading. | Scaffold |
| 22 | `Instancing` | Draw many copies of a mesh with per-instance transform or color data. | Scaffold |
| 23 | `Shadow Mapping` | Render depth from a light view and sample it when shading the main scene. | Scaffold |
| 24 | `Geometry Shader` | Use a geometry shader for controlled primitive expansion or visualization. | Scaffold |
| 25 | `Compute Shader` | Dispatch a compute shader and synchronize its result for rendering or readback. | Scaffold |
| 26 | `ImGui Integration` | Attach Dear ImGui as a debug UI overlay for OpenGL runtime parameters. | Scaffold |
| 27 | `Mini Renderer` | Combine context setup, shader management, scene data, render passes, and debug UI into a small renderer. | Scaffold |

## Implementation Strategy

`01. WglBasic` through `05. Index Buffer` are implemented as the foundation. Later samples remain scaffolded until they are implemented in order, with each step adding exactly one primary OpenGL concept.

## Next Implementation Pass

Start with `06. Vertex Array Object` next:

- keep the WGL shell, shader program, VBO, and index buffer from 05
- introduce VAO creation, binding, and deletion
- move vertex-array state setup out of the per-frame render path
