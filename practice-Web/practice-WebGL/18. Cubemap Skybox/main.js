'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement TEXTURE_CUBE_MAP skybox
// A cubemap is a texture with 6 faces (+X, -X, +Y, -Y, +Z, -Z).
// Sampling uses a 3D direction vector instead of 2D UVs.
// The skybox trick: render a cube that always surrounds the camera, with
// normals pointing inward, and write depth = 1.0 so it stays behind everything.
//
// Key steps:
//   1. Create 6 solid-color face textures and upload them to gl.TEXTURE_CUBE_MAP
//   2. Render a unit cube with inverted winding; in VS strip translation from
//      the view matrix (mat3→mat4) so the cube follows camera rotation only
//   3. Set gl.depthFunc(gl.LEQUAL) and write gl_Position.z = gl_Position.w
//      in the VS to force depth = 1.0 (always at far plane)
//
// Mouse drag → rotate yaw/pitch of the camera.

// ----- Skybox vertex shader -----
// Strip translation from the view matrix: use only the 3×3 rotation part.
// This means the cube always appears infinitely far away.
const SKY_VS = `#version 300 es
in vec3 a_pos;
uniform mat4 u_proj;
uniform mat4 u_view;
out vec3 v_dir;
void main() {
    v_dir = a_pos;  // direction vector = cube vertex position (unit cube)
    // Use only the rotation part of the view matrix (top-left 3×3)
    mat4 rotView = mat4(mat3(u_view));
    vec4 clip = u_proj * rotView * vec4(a_pos, 1.0);
    // Force z = w so that after perspective divide z/w = 1.0 (far plane).
    // gl.LEQUAL depth test lets it pass when scene depth = 1.0.
    gl_Position = clip.xyww;
}`;

// ----- Skybox fragment shader -----
const SKY_FS = `#version 300 es
precision mediump float;
uniform samplerCube u_skybox;
in  vec3 v_dir;
out vec4 outColor;
void main() {
    outColor = texture(u_skybox, v_dir);
}`;

// ----- Cube vertex positions (unit cube, 36 vertices) -----
// Each face = 2 triangles. Winding is standard (normals outward for a skybox
// the camera is inside, so outward normals face the camera correctly).
const cubeVerts = new Float32Array([
    -1, 1,-1, -1,-1,-1,  1,-1,-1,  1,-1,-1,  1, 1,-1, -1, 1,-1,
    -1,-1, 1, -1,-1,-1, -1, 1,-1, -1, 1,-1, -1, 1, 1, -1,-1, 1,
     1,-1,-1,  1,-1, 1,  1, 1, 1,  1, 1, 1,  1, 1,-1,  1,-1,-1,
    -1,-1, 1, -1, 1, 1,  1, 1, 1,  1, 1, 1,  1,-1, 1, -1,-1, 1,
    -1, 1,-1,  1, 1,-1,  1, 1, 1,  1, 1, 1, -1, 1, 1, -1, 1,-1,
    -1,-1,-1, -1,-1, 1,  1,-1,-1,  1,-1,-1, -1,-1, 1,  1,-1, 1,
]);

// TODO Step 1: Create cubemap texture with 6 distinctly-colored 4×4 faces
function makeFacePixels(r, g, b) {
    const d = new Uint8Array(4 * 4 * 4);
    for (let i = 0; i < 16; i++) { d[i*4]=r; d[i*4+1]=g; d[i*4+2]=b; d[i*4+3]=255; }
    return d;
}
// Face order matches gl.TEXTURE_CUBE_MAP_POSITIVE_X + 0..5
const faceData = [
    makeFacePixels(200,  50,  50),  // +X  red
    makeFacePixels(100,  50, 200),  // -X  blue
    makeFacePixels( 50, 180,  50),  // +Y  green
    makeFacePixels(180, 100,  50),  // -Y  orange
    makeFacePixels(180, 180,  50),  // +Z  yellow
    makeFacePixels( 50, 180, 180),  // -Z  cyan
];

// TODO Step 2: Upload cubemap faces
const cubeTex = gl.createTexture();
gl.bindTexture(gl.TEXTURE_CUBE_MAP, cubeTex);
for (let i = 0; i < 6; i++) {
    // gl.TEXTURE_CUBE_MAP_POSITIVE_X is the base enum; the 6 faces are
    // consecutive integers: +X, -X, +Y, -Y, +Z, -Z
    gl.texImage2D(
        gl.TEXTURE_CUBE_MAP_POSITIVE_X + i,
        0, gl.RGBA, 4, 4, 0,
        gl.RGBA, gl.UNSIGNED_BYTE, faceData[i]
    );
}
gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
// Clamp to edge prevents seams at cube face boundaries
gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_R, gl.CLAMP_TO_EDGE);

// ----- Build VAO -----
const vao = gl.createVertexArray();
gl.bindVertexArray(vao);
const vbo = gl.createBuffer();
gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
gl.bufferData(gl.ARRAY_BUFFER, cubeVerts, gl.STATIC_DRAW);
gl.enableVertexAttribArray(0);
gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 0, 0);

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
gl.attachShader(prog, compileShader(gl.VERTEX_SHADER,   SKY_VS));
gl.attachShader(prog, compileShader(gl.FRAGMENT_SHADER, SKY_FS));
gl.bindAttribLocation(prog, 0, 'a_pos');
gl.linkProgram(prog);

const uProj    = gl.getUniformLocation(prog, 'u_proj');
const uView    = gl.getUniformLocation(prog, 'u_view');
const uSkybox  = gl.getUniformLocation(prog, 'u_skybox');

// ----- Camera state (mouse drag) -----
let yaw = 0, pitch = 0, lastX = 0, lastY = 0, dragging = false;
canvas.addEventListener('mousedown', e => { dragging = true; lastX = e.clientX; lastY = e.clientY; });
canvas.addEventListener('mouseup',   () => { dragging = false; });
canvas.addEventListener('mousemove', e => {
    if (!dragging) return;
    yaw   += (e.clientX - lastX) * 0.4;
    pitch += (e.clientY - lastY) * 0.4;
    pitch  = Math.max(-89, Math.min(89, pitch));
    lastX = e.clientX; lastY = e.clientY;
});

// Simple mat4 helpers (no dependency on math.js for self-contained learning)
function perspective(fov, asp, near, far) {
    const f = 1 / Math.tan(fov * Math.PI / 360);
    const d = near - far;
    return new Float32Array([
        f/asp, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (far+near)/d, -1,
        0, 0, (2*far*near)/d, 0
    ]);
}
function rotY(deg) {
    const r = deg * Math.PI / 180;
    const c = Math.cos(r), s = Math.sin(r);
    return new Float32Array([c,0,-s,0, 0,1,0,0, s,0,c,0, 0,0,0,1]);
}
function rotX(deg) {
    const r = deg * Math.PI / 180;
    const c = Math.cos(r), s = Math.sin(r);
    return new Float32Array([1,0,0,0, 0,c,s,0, 0,-s,c,0, 0,0,0,1]);
}
function mulMat4(a, b) {
    const out = new Float32Array(16);
    for (let i = 0; i < 4; i++)
        for (let j = 0; j < 4; j++)
            for (let k = 0; k < 4; k++)
                out[i*4+j] += a[i*4+k] * b[k*4+j];
    return out;
}

const proj = perspective(60, canvas.width / canvas.height, 0.1, 100);

// TODO Step 3: Render with LEQUAL depth — skybox at far plane
function frame() {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);
    // LEQUAL allows fragments with depth == 1.0 (from the skybox) to pass
    gl.depthFunc(gl.LEQUAL);

    const view = mulMat4(rotX(pitch), rotY(yaw));

    gl.useProgram(prog);
    gl.uniformMatrix4fv(uProj,   false, proj);
    gl.uniformMatrix4fv(uView,   false, view);
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_CUBE_MAP, cubeTex);
    gl.uniform1i(uSkybox, 0);

    gl.bindVertexArray(vao);
    gl.drawArrays(gl.TRIANGLES, 0, 36);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
