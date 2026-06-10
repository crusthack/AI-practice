# practice-WebGL

WebGL2 graphics programming — 27 self-contained browser samples that follow the same step-by-step progression as `practice-DirectX11`, `practice-OpenGL`, and `practice-Vulkan`.

## Requirements

- Any modern browser with WebGL2 support (Chrome 56+, Firefox 51+, Edge 79+).
- A local HTTP server to avoid CORS issues when samples load external assets.

```powershell
# Quick server — Python (no install required on most systems)
python -m http.server 8080

# Or Node.js
npx serve .
```

Then open `http://localhost:8080/01. WebGL Context/index.html`.

> Modules 01–05 use only inline JavaScript and have no external asset dependencies; they can be opened directly as `file://` URLs.

## Directory Layout

```
practice-WebGL/
├── common/
│   ├── webgl-utils.js   # createProgram, createBuffer, createTexture2D, getUniformLocations
│   └── math.js          # Vec3, Mat4 — column-major, Float32Array-based
├── docs/
│   └── webgl-reference.md
├── 01. WebGL Context/   # Implemented
├── 02. Hello Triangle/  # Implemented
├── 03. Shader Program/  # Implemented
├── 04. Vertex Buffer/   # Implemented
├── 05. Index Buffer/    # Implemented
├── 06. Vertex Array Object/ … 27. Mini Renderer/   # Scaffold
├── ROADMAP.md
└── WORKSPACE_CONTEXT.md
```

## Key Differences from Desktop OpenGL

| Topic | Desktop OpenGL (C++) | WebGL2 (JS/Browser) |
|---|---|---|
| Context creation | WGL / GLX | `canvas.getContext('webgl2')` |
| Shader language | GLSL 4.x | GLSL ES 3.00 (`#version 300 es`) |
| Buffer management | Same API | Same API |
| Compute shaders | Available | **Not available** (use Transform Feedback / MRT instead) |
| Extensions | `glGetString(GL_EXTENSIONS)` | `gl.getExtension('EXT_...')` |
| Floating-point textures | Default | Requires `EXT_color_buffer_float` |
| VAO | Core since GL 3.0 | Core in WebGL2 |

## Module Status

See [ROADMAP.md](ROADMAP.md) for the full list and implementation status.
