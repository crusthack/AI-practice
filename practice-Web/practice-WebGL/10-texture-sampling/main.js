'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement 2D texture sampling with a procedural checkerboard texture
// No image files needed — generate the pixel data in JS and upload to the GPU.
//
// Key steps:
//   1. Generate an NxN RGBA Uint8Array procedurally.
//      Checkerboard: if ((x ^ y) & 1) use color A else color B.
//      (Also try: radial gradient, UV-colored grid, noise pattern)
//   2. Create a texture object and upload:
//        const tex = gl.createTexture();
//        gl.bindTexture(gl.TEXTURE_2D, tex);
//        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, N, N, 0, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
//        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST); // no mipmap blur
//        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
//        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
//        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
//   3. In the fragment shader sample with: texture(u_tex, v_uv)
//      Bind before draw: gl.activeTexture(gl.TEXTURE0); gl.bindTexture(TEXTURE_2D, tex);
//      Set sampler uniform: gl.uniform1i(loc_tex, 0);  // 0 = texture unit 0

// --- Shaders ---
const vsSource = `#version 300 es
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;
out vec2 v_uv;
void main() {
    v_uv = a_uv;
    gl_Position = vec4(a_position, 0.0, 1.0);
}`;

const fsSource = `#version 300 es
precision mediump float;
in vec2 v_uv;
uniform sampler2D u_tex;
out vec4 fragColor;
void main() {
    // TODO: sample the texture
    // fragColor = texture(u_tex, v_uv);
    fragColor = vec4(v_uv, 0.0, 1.0); // placeholder: visualise UVs as color
}`;

// TODO: 1. Compile shaders, link program, get loc_tex = gl.getUniformLocation(program, 'u_tex')

// TODO: 2. Generate procedural texture (64x64 checkerboard)
// const N = 64;
// const pixels = new Uint8Array(N * N * 4);
// for (let y = 0; y < N; y++) {
//     for (let x = 0; x < N; x++) {
//         const i = (y * N + x) * 4;
//         const checker = ((x >> 3) ^ (y >> 3)) & 1; // 8x8 cells
//         pixels[i]   = checker ? 255 : 30;   // R
//         pixels[i+1] = checker ? 255 : 30;   // G
//         pixels[i+2] = checker ? 255 : 200;  // B
//         pixels[i+3] = 255;                  // A
//     }
// }

// TODO: 3. Upload texture (see steps above) and create fullscreen quad VAO with UV coords
// Quad UV: (0,0) bottom-left ... (1,1) top-right; remember WebGL texture origin is bottom-left.

function frame(now) {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    // TODO: bind texture to unit 0, set sampler uniform, draw quad
    // gl.activeTexture(gl.TEXTURE0);
    // gl.bindTexture(gl.TEXTURE_2D, tex);
    // gl.useProgram(program);
    // gl.uniform1i(loc_tex, 0);
    // gl.bindVertexArray(vao);
    // gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
