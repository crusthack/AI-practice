# 14 - Framebuffer Object

## Goal

Render a 3D scene into an off-screen FBO (render-to-texture), then display that texture on a fullscreen quad in a second pass — establishing the two-pass pipeline that all post-processing effects depend on.

## Key API Calls / Concepts

| Item | Description |
|---|---|
| `gl.createFramebuffer()` | Allocates an off-screen render target object |
| `gl.framebufferTexture2D(FRAMEBUFFER, COLOR_ATTACHMENT0, TEXTURE_2D, tex, 0)` | Attaches a texture as the color output of the FBO |
| `gl.framebufferRenderbuffer(FRAMEBUFFER, DEPTH_ATTACHMENT, RENDERBUFFER, rb)` | Attaches a depth renderbuffer so depth test works during pass 1 |
| `gl.checkFramebufferStatus(FRAMEBUFFER)` | Returns `FRAMEBUFFER_COMPLETE` if the FBO is valid and ready to use |
| `gl.bindFramebuffer(FRAMEBUFFER, null)` | Restores rendering to the default (canvas) framebuffer |

## Implementation Steps

1. Create `fboTex` (RGBA, W×H, `CLAMP_TO_EDGE`), create `depthRB` (`DEPTH_COMPONENT24`), attach both to `fbo`, verify `checkFramebufferStatus === FRAMEBUFFER_COMPLETE`.
2. Pass 1 each frame: bind `fbo`, clear, draw the 3D scene with `sceneProgram`, unbind `fbo`.
3. Pass 2 each frame: restore default framebuffer, bind `fboTex` to texture unit 0, draw a fullscreen quad with `quadProgram` sampling that texture.

## Expected Output

The 3D scene (spinning cube) is displayed fullscreen — identical to module 08 visually, but now the pixels travel through an FBO, proving the render-to-texture pipeline is functional and ready for post-processing.
