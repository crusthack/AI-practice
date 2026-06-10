'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement a Framebuffer Object (FBO) for render-to-texture
// An FBO is an off-screen render target. You render your scene into it,
// then sample its color attachment as a texture in a second "fullscreen quad" pass.
// This is the foundation for ALL post-processing effects.
//
// Key steps:
//   1. Create the FBO and its color attachment texture:
//        const fboTex = gl.createTexture();
//        gl.bindTexture(gl.TEXTURE_2D, fboTex);
//        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, W, H, 0, gl.RGBA, gl.UNSIGNED_BYTE, null); // null = allocate only
//        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
//        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
//        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
//        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
//
//        Create a depth renderbuffer too (needed if depth test runs in pass 1):
//        const depthRB = gl.createRenderbuffer();
//        gl.bindRenderbuffer(gl.RENDERBUFFER, depthRB);
//        gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT24, W, H);
//
//        const fbo = gl.createFramebuffer();
//        gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
//        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, fboTex, 0);
//        gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, depthRB);
//
//        Verify: gl.checkFramebufferStatus(gl.FRAMEBUFFER) === gl.FRAMEBUFFER_COMPLETE
//        gl.bindFramebuffer(gl.FRAMEBUFFER, null); // unbind
//
//   2. First pass (render scene to FBO):
//        gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
//        gl.viewport(0, 0, W, H);
//        gl.clear(...);
//        // draw your 3D scene here
//        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
//
//   3. Second pass (display FBO texture on fullscreen quad):
//        gl.viewport(0, 0, canvas.width, canvas.height);
//        gl.clear(gl.COLOR_BUFFER_BIT);
//        gl.activeTexture(gl.TEXTURE0);
//        gl.bindTexture(gl.TEXTURE_2D, fboTex);
//        gl.useProgram(quadProgram);
//        gl.uniform1i(loc_tex, 0);
//        gl.drawElements(...); // fullscreen quad

const W = 800, H = 600; // FBO dimensions (match canvas for 1:1 pixels)

// TODO: 1. Create fboTex, depthRB, fbo as described above.
//          Check gl.checkFramebufferStatus returns FRAMEBUFFER_COMPLETE.

// --- Scene shader (pass 1) ---
const sceneVS = `#version 300 es
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;
uniform mat4 u_mvp;
out vec3 v_color;
void main() { v_color = a_color; gl_Position = u_mvp * vec4(a_position, 1.0); }`;

const sceneFS = `#version 300 es
precision mediump float;
in vec3 v_color;
out vec4 fragColor;
void main() { fragColor = vec4(v_color, 1.0); }`;

// --- Fullscreen quad shader (pass 2) ---
const quadVS = `#version 300 es
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;
out vec2 v_uv;
void main() { v_uv = a_uv; gl_Position = vec4(a_position, 0.0, 1.0); }`;

const quadFS = `#version 300 es
precision mediump float;
in vec2 v_uv;
uniform sampler2D u_tex;
out vec4 fragColor;
void main() {
    // TODO: sample the FBO texture — this IS the scene rendered offscreen
    fragColor = texture(u_tex, v_uv);
}`;

// TODO: 2. Compile sceneProgram and quadProgram separately.
//          Build: cube VAO for scene, fullscreen quad VAO for pass 2.

function frame(now) {
    const t = now / 1000.0;

    // --- Pass 1: render scene into FBO ---
    // TODO:
    // gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
    // gl.viewport(0, 0, W, H);
    // gl.clearColor(0.05, 0.05, 0.10, 1.0);
    // gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    // gl.enable(gl.DEPTH_TEST);
    // ... draw spinning cube with sceneProgram ...
    // gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    // --- Pass 2: blit FBO texture to screen ---
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);
    // TODO:
    // gl.disable(gl.DEPTH_TEST);
    // gl.activeTexture(gl.TEXTURE0);
    // gl.bindTexture(gl.TEXTURE_2D, fboTex);
    // gl.useProgram(quadProgram);
    // gl.uniform1i(loc_tex, 0);
    // gl.bindVertexArray(quadVAO);
    // gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
