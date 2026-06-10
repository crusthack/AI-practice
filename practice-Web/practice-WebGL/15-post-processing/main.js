'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement post-processing effects using the FBO pipeline from module 14
// Pass 1 renders the scene to an FBO texture (same as module 14).
// Pass 2 applies a screen-space effect in the fragment shader:
//   Effect 0 — none    (passthrough, same as module 14)
//   Effect 1 — grayscale: luma = dot(color.rgb, vec3(0.299, 0.587, 0.114))
//   Effect 2 — invert:    vec3(1.0) - color.rgb
//   Effect 3 — vignette: darken pixels by distance from screen center
//
// Key steps:
//   1. Reuse the FBO setup from module 14 (fboTex + depthRB + fbo).
//   2. Write a post-process fragment shader with a uniform int u_effect (0-3).
//      Use if/else or a switch on u_effect to select the active effect.
//      Keep all effects in ONE shader to avoid program switches per frame.
//   3. Listen for keys 1-4 to set the current effect index; upload it each frame
//      with gl.uniform1i(loc_effect, effectIndex).

let effectIndex = 0; // 0=none, 1=grayscale, 2=invert, 3=vignette

document.addEventListener('keydown', e => {
    const map = { '1': 0, '2': 1, '3': 2, '4': 3 };
    if (e.key in map) {
        effectIndex = map[e.key];
        const names = ['none', 'grayscale', 'invert', 'vignette'];
        document.getElementById('info').textContent =
            `15 - Post Processing | effect: ${names[effectIndex]} (keys 1-4)`;
    }
});

const W = 800, H = 600;

// --- Scene shader (pass 1) — identical to module 14 ---
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

// --- Post-process shader (pass 2) ---
const postVS = `#version 300 es
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;
out vec2 v_uv;
void main() { v_uv = a_uv; gl_Position = vec4(a_position, 0.0, 1.0); }`;

// TODO: implement all 4 effects in the fragment shader
const postFS = `#version 300 es
precision mediump float;
in vec2 v_uv;
uniform sampler2D u_tex;
uniform int u_effect;   // 0=none, 1=grayscale, 2=invert, 3=vignette
out vec4 fragColor;

void main() {
    vec4 color = texture(u_tex, v_uv);

    // TODO: implement effect selection
    // if (u_effect == 1) {
    //     float luma = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    //     color.rgb = vec3(luma);
    // } else if (u_effect == 2) {
    //     color.rgb = vec3(1.0) - color.rgb;
    // } else if (u_effect == 3) {
    //     vec2 uv = v_uv - 0.5;           // center at origin
    //     float vignette = 1.0 - dot(uv, uv) * 3.0;
    //     color.rgb *= clamp(vignette, 0.0, 1.0);
    // }

    fragColor = color;
}`;

// TODO: 1. Create FBO (fbo + fboTex + depthRB) — same as module 14.
// TODO: 2. Compile sceneProgram and postProgram; cache loc_mvp, loc_tex, loc_effect.
// TODO: 3. Create cube VAO for scene pass and fullscreen quad VAO for post pass.

function frame(now) {
    const t = now / 1000.0;

    // --- Pass 1: render scene to FBO ---
    // TODO: bind fbo, clear, draw scene, unbind fbo

    // --- Pass 2: apply post-process effect ---
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.disable(gl.DEPTH_TEST);

    // TODO:
    // gl.activeTexture(gl.TEXTURE0);
    // gl.bindTexture(gl.TEXTURE_2D, fboTex);
    // gl.useProgram(postProgram);
    // gl.uniform1i(loc_tex,    0);
    // gl.uniform1i(loc_effect, effectIndex);
    // gl.bindVertexArray(quadVAO);
    // gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
