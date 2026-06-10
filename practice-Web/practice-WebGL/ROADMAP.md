# WebGL Learning Roadmap

27-step roadmap aligned with the DirectX11/OpenGL practice sets. The sequence follows the WebGL2 rendering pipeline: context setup → geometry → shaders → transforms → textures → framebuffers → advanced GPU techniques.

## Learning Principles

- One sample, one primary concept.
- Run every sample by opening `index.html` in a browser (no build step required).
- Keep the WebGL2 canvas setup explicit in every `main.js`.
- Use `../common/webgl-utils.js` and `../common/math.js` for utilities.
- Prefer procedural data before asset-loading samples.
- Every sample must produce a visible result that can be verified quickly.

## Full Sequence

| No. | Sample | Goal | Status |
|---|---|---|---|
| 01 | `WebGL Context` | Create a canvas, initialize a WebGL2 rendering context, animate the clear color, and build a requestAnimationFrame loop. | Implemented |
| 02 | `Hello Triangle` | Draw the first triangle using `gl_VertexID` (no VBO), a minimal vertex shader, and a fixed fragment color. | Implemented |
| 03 | `Shader Program` | Compile GLSL ES vertex/fragment shaders with explicit error reporting, link a program, cache uniform locations. | Implemented |
| 04 | `Vertex Buffer` | Upload interleaved vertex data (position + color) into a VBO and configure multiple vertex attributes. | Implemented |
| 05 | `Index Buffer` | Reuse vertices through an element buffer object (EBO) and `drawElements`. | Implemented |
| 06 | `Vertex Array Object` | Encapsulate vertex attribute state in a VAO and separate object setup from per-frame draw calls. | Scaffold |
| 07 | `Uniforms And Time` | Animate geometry and color using `u_time` and multiple uniform types (`float`, `vec2`, `vec4`). | Scaffold |
| 08 | `Transform Matrices` | Send model, view, and projection matrices to GLSL and transform 3D geometry with `mat4`. | Scaffold |
| 09 | `Camera` | Build an orbit camera driven by mouse drag; update the view matrix every frame. | Scaffold |
| 10 | `Texture Sampling` | Create a procedural RGBA texture, configure sampler parameters, and sample it in GLSL. | Scaffold |
| 11 | `Depth Buffer` | Enable depth testing so nearer fragments occlude farther fragments in a 3D scene. | Scaffold |
| 12 | `Face Culling` | Control front-face winding order and enable back-face culling. | Scaffold |
| 13 | `Blending` | Enable alpha blending and render transparent quads in back-to-front order. | Scaffold |
| 14 | `Framebuffer Object` | Render a scene into an offscreen FBO color attachment, then blit it onto a fullscreen quad. | Scaffold |
| 15 | `Post Processing` | Apply a simple post-process effect (grayscale / vignette) in a second fullscreen pass. | Scaffold |
| 16 | `Uniform Buffer Object` | Group per-frame constants in a UBO and share them across multiple shader programs. | Scaffold |
| 17 | `Texture Array` | Store multiple related textures in a `TEXTURE_2D_ARRAY` and select layers in the fragment shader. | Scaffold |
| 18 | `Cubemap Skybox` | Create a cubemap texture and render a skybox, disabling depth writes for the background. | Scaffold |
| 19 | `Model Loading` | Parse a simple JSON mesh format, upload positions/normals/UVs, and draw indexed geometry. | Scaffold |
| 20 | `Scene Graph` | Render multiple objects with a parent–child transform hierarchy using recursive matrix multiplication. | Scaffold |
| 21 | `Lighting And Materials` | Combine vertex normals, material parameters, and a point light for Blinn–Phong shading. | Scaffold |
| 22 | `Instancing` | Draw thousands of meshes in a single call using `drawArraysInstanced` and per-instance data. | Scaffold |
| 23 | `Shadow Mapping` | Render depth from a light's perspective into a depth texture and sample it in the main pass. | Scaffold |
| 24 | `Transform Feedback` | Capture vertex shader output into a buffer via WebGL2 transform feedback for GPU-side simulation. | Scaffold |
| 25 | `Multi-Render Target` | Write to multiple color attachments in one pass and read them back in a subsequent pass. | Scaffold |
| 26 | `Stats And Debug` | Overlay a per-frame stats HUD (FPS, draw calls, triangle count) and add shader hot-reload. | Scaffold |
| 27 | `Mini Renderer` | Combine context setup, shader management, a scene graph, render passes, and the stats overlay into a small self-contained renderer. | Scaffold |

## Implementation Strategy

Modules `01`–`05` are fully implemented and establish the core pipeline. Later modules are scaffolded until implemented in order, each adding exactly one primary WebGL concept.

## Next Implementation Pass

Start with `06. Vertex Array Object`:
- Reuse the VBO + shader from `05`
- Extract attribute setup into `setupVAO(gl, prog, vbo, ebo)` 
- Demonstrate binding a VAO for draw vs. unbinding for setup
