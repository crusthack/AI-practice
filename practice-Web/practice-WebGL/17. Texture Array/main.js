'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement TEXTURE_2D_ARRAY
// A TEXTURE_2D_ARRAY stores N same-sized 2D images as indexed "layers" in a
// single texture object. The GPU can sample any layer at runtime without
// binding a different texture, which is very efficient.
//
// Key steps:
//   1. Generate 4 distinct 4×4 pixel arrays (e.g. solid red, green, blue, yellow)
//   2. Allocate storage with gl.texImage3D(TEXTURE_2D_ARRAY, 0, gl.RGBA, W, H, LAYERS, ...)
//      then upload each layer with gl.texSubImage3D(..., layer, ...)
//   3. In the fragment shader declare `uniform sampler2DArray u_tex`,
//      and sample as `texture(u_tex, vec3(uv, u_layer))` where u_layer is 0-3.
//   4. Press keys 0-3 to update the u_layer uniform and see each layer.
//
// Why TEXTURE_2D_ARRAY?  Avoids texture-atlas UV math, keeps filtering correct
// at layer boundaries, and allows dynamic layer selection in the shader.

// ----- Vertex shader -----
const VS_SRC = `#version 300 es
in vec2 a_pos;
out vec2 v_uv;
void main() {
    // Full-screen quad: a_pos in [-1,1] → UV in [0,1]
    v_uv = a_pos * 0.5 + 0.5;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}`;

// ----- Fragment shader -----
const FS_SRC = `#version 300 es
precision mediump float;
uniform sampler2DArray u_tex;   // TEXTURE_2D_ARRAY bound to unit 0
uniform float u_layer;          // which layer to display (0–3)
in  vec2 v_uv;
out vec4 outColor;
void main() {
    // vec3 UV: xy are the 2D coordinates, z selects the layer index.
    // The z component is automatically rounded to the nearest integer layer.
    outColor = texture(u_tex, vec3(v_uv, u_layer));
}`;

// TODO Step 1: Build pixel data for 4 layers (4×4 RGBA each)
function makeLayerPixels(r, g, b) {
    const data = new Uint8Array(4 * 4 * 4); // 4×4 pixels, 4 channels
    for (let i = 0; i < 16; i++) {
        data[i * 4 + 0] = r;
        data[i * 4 + 1] = g;
        data[i * 4 + 2] = b;
        data[i * 4 + 3] = 255;
    }
    return data;
}
const layers = [
    makeLayerPixels(220,  60,  60),  // layer 0 – red
    makeLayerPixels( 60, 200,  60),  // layer 1 – green
    makeLayerPixels( 60, 100, 220),  // layer 2 – blue
    makeLayerPixels(220, 200,  50),  // layer 3 – yellow
];
const TEX_W = 4, TEX_H = 4, NUM_LAYERS = 4;

// TODO Step 2: Create and upload TEXTURE_2D_ARRAY
const texArray = gl.createTexture();
gl.bindTexture(gl.TEXTURE_2D_ARRAY, texArray);

// Allocate all layers at once via texImage3D.
// Signature: texImage3D(target, level, internalFormat, width, height, depth,
//                       border, format, type, data|null)
// depth = number of layers.
gl.texImage3D(
    gl.TEXTURE_2D_ARRAY, 0, gl.RGBA8,
    TEX_W, TEX_H, NUM_LAYERS,
    0,                        // border – must be 0
    gl.RGBA, gl.UNSIGNED_BYTE,
    null                      // allocate without data, fill per-layer below
);

for (let i = 0; i < NUM_LAYERS; i++) {
    // texSubImage3D lets us upload one layer at a time.
    // The 6th parameter (zOffset) is the layer index.
    gl.texSubImage3D(
        gl.TEXTURE_2D_ARRAY, 0,
        0, 0, i,               // xOffset, yOffset, zOffset (layer)
        TEX_W, TEX_H, 1,       // width, height, depth (1 layer)
        gl.RGBA, gl.UNSIGNED_BYTE, layers[i]
    );
}
gl.texParameteri(gl.TEXTURE_2D_ARRAY, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
gl.texParameteri(gl.TEXTURE_2D_ARRAY, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

// ----- Fullscreen quad geometry -----
const quadVerts = new Float32Array([-1,-1, 1,-1, -1,1, 1,1]);
const vao = gl.createVertexArray();
gl.bindVertexArray(vao);
const vbo = gl.createBuffer();
gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
gl.bufferData(gl.ARRAY_BUFFER, quadVerts, gl.STATIC_DRAW);
gl.enableVertexAttribArray(0);
gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 0, 0);

// ----- Compile program -----
function compileShader(type, src) {
    const s = gl.createShader(type);
    gl.shaderSource(s, src);
    gl.compileShader(s);
    if (!gl.getShaderParameter(s, gl.COMPILE_STATUS))
        throw new Error(gl.getShaderInfoLog(s));
    return s;
}
const prog = gl.createProgram();
gl.attachShader(prog, compileShader(gl.VERTEX_SHADER,   VS_SRC));
gl.attachShader(prog, compileShader(gl.FRAGMENT_SHADER, FS_SRC));
gl.bindAttribLocation(prog, 0, 'a_pos');
gl.linkProgram(prog);

const uTex   = gl.getUniformLocation(prog, 'u_tex');
const uLayer = gl.getUniformLocation(prog, 'u_layer');

// TODO Step 3: Keyboard handler — press 0-3 to switch layers
let currentLayer = 0;
window.addEventListener('keydown', e => {
    const n = parseInt(e.key);
    if (n >= 0 && n < NUM_LAYERS) currentLayer = n;
});

function frame() {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(prog);

    // Bind the TEXTURE_2D_ARRAY to texture unit 0
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D_ARRAY, texArray);
    gl.uniform1i(uTex, 0);

    // Pass the selected layer index to the shader
    gl.uniform1f(uLayer, currentLayer);

    gl.bindVertexArray(vao);
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
