'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement mesh loading (procedural sphere) with normals and diffuse lighting
// Real model loading (OBJ/glTF) follows the same pattern: parse the file into
// flat arrays of positions, normals, UVs, and indices, then upload to GPU.
//
// Key steps:
//   1. Build a UV sphere mesh: loop over latitude/longitude, compute position,
//      normal (= normalize(position) for a unit sphere), and UV, then create
//      index buffer for the triangle strip pairs.
//   2. Upload as interleaved VBO: [x,y,z, nx,ny,nz, u,v] per vertex.
//      Use stride = 8 * sizeof(float), with different offsets per attribute.
//   3. In the fragment shader: diffuse = max(dot(N, L), 0.0) * lightColor.
//      The normal matrix (inverse-transpose of modelView's 3×3) must be used
//      to correctly transform normals when the model is non-uniformly scaled.

// ----- Shaders -----
const VS = `#version 300 es
layout(location=0) in vec3 a_pos;
layout(location=1) in vec3 a_normal;
layout(location=2) in vec2 a_uv;
uniform mat4 u_mvp;
uniform mat3 u_normalMat;  // transpose(inverse(modelView 3×3)) for correct normal transform
out vec3 v_normal;
out vec3 v_pos;
out vec2 v_uv;
void main() {
    v_normal = normalize(u_normalMat * a_normal);
    v_pos    = a_pos;
    v_uv     = a_uv;
    gl_Position = u_mvp * vec4(a_pos, 1.0);
}`;

const FS = `#version 300 es
precision mediump float;
in vec3 v_normal;
in vec3 v_pos;
in vec2 v_uv;
out vec4 outColor;
void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.5, 2.0));
    float diffuse  = max(dot(normalize(v_normal), lightDir), 0.0);
    vec3  baseColor = vec3(0.3, 0.6, 0.9);
    vec3  color = baseColor * (0.15 + 0.85 * diffuse);
    outColor = vec4(color, 1.0);
}`;

// ----- TODO Step 1: Build UV sphere mesh -----
function buildSphere(stacks, slices) {
    const verts   = [];  // interleaved: pos(3) + normal(3) + uv(2)
    const indices = [];

    for (let i = 0; i <= stacks; i++) {
        const phi = Math.PI * i / stacks;        // 0 → π
        const y   = Math.cos(phi);
        const r   = Math.sin(phi);

        for (let j = 0; j <= slices; j++) {
            const theta = 2 * Math.PI * j / slices;  // 0 → 2π
            const x = r * Math.cos(theta);
            const z = r * Math.sin(theta);
            const u = j / slices;
            const v = i / stacks;

            // For a unit sphere, the normal equals the position vector
            verts.push(x, y, z,   x, y, z,   u, v);
        }
    }

    for (let i = 0; i < stacks; i++) {
        for (let j = 0; j < slices; j++) {
            const a = i * (slices + 1) + j;
            const b = a + (slices + 1);
            // Two triangles per quad
            indices.push(a, b, a+1,  b, b+1, a+1);
        }
    }

    return {
        vertices: new Float32Array(verts),
        indices:  new Uint16Array(indices),
        count:    indices.length
    };
}

const mesh = buildSphere(32, 32);

// TODO Step 2: Upload interleaved VBO + index buffer
const vao = gl.createVertexArray();
gl.bindVertexArray(vao);

const vbo = gl.createBuffer();
gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
gl.bufferData(gl.ARRAY_BUFFER, mesh.vertices, gl.STATIC_DRAW);

// Interleaved layout: [x,y,z, nx,ny,nz, u,v]
const STRIDE = 8 * 4;  // 8 floats × 4 bytes
// Attribute 0: position (offset 0)
gl.enableVertexAttribArray(0);
gl.vertexAttribPointer(0, 3, gl.FLOAT, false, STRIDE, 0);
// Attribute 1: normal (offset 3 floats = 12 bytes)
gl.enableVertexAttribArray(1);
gl.vertexAttribPointer(1, 3, gl.FLOAT, false, STRIDE, 12);
// Attribute 2: UV (offset 6 floats = 24 bytes)
gl.enableVertexAttribArray(2);
gl.vertexAttribPointer(2, 2, gl.FLOAT, false, STRIDE, 24);

const ibo = gl.createBuffer();
gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, mesh.indices, gl.STATIC_DRAW);

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
gl.attachShader(prog, compileShader(gl.VERTEX_SHADER,   VS));
gl.attachShader(prog, compileShader(gl.FRAGMENT_SHADER, FS));
gl.linkProgram(prog);

const uMVP       = gl.getUniformLocation(prog, 'u_mvp');
const uNormalMat = gl.getUniformLocation(prog, 'u_normalMat');

// TODO Step 3: Compute MVP + normal matrix each frame
// Simple perspective and rotation helpers
function perspective(fov, asp, n, f) {
    const t = Math.tan(fov * Math.PI / 360);
    return new Float32Array([
        1/(asp*t),0,0,0,  0,1/t,0,0,
        0,0,(f+n)/(n-f),-1,  0,0,(2*f*n)/(n-f),0
    ]);
}
function rotY(t) {
    const c=Math.cos(t),s=Math.sin(t);
    return new Float32Array([c,0,-s,0, 0,1,0,0, s,0,c,0, 0,0,0,1]);
}
function translate(x,y,z) {
    return new Float32Array([1,0,0,0, 0,1,0,0, 0,0,1,0, x,y,z,1]);
}
function mul(a,b) {
    const o=new Float32Array(16);
    for(let i=0;i<4;i++) for(let j=0;j<4;j++) for(let k=0;k<4;k++) o[i*4+j]+=a[i*4+k]*b[k*4+j];
    return o;
}
// Extract 3×3 from mat4 (column-major, no translation) – identity is fine for
// a uniformly scaled model; for production use the actual inverse-transpose.
function normalMatrix(m4) {
    return new Float32Array([m4[0],m4[1],m4[2], m4[4],m4[5],m4[6], m4[8],m4[9],m4[10]]);
}

const proj = perspective(45, canvas.width/canvas.height, 0.1, 100);
const cam  = translate(0, 0, -3);

function frame(now) {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);

    const angle  = now * 0.001;
    const model  = rotY(angle);
    const mv     = mul(cam, model);
    const mvp    = mul(proj, mv);
    const normMat = normalMatrix(mv);

    gl.useProgram(prog);
    gl.uniformMatrix4fv(uMVP,       false, mvp);
    gl.uniformMatrix3fv(uNormalMat, false, normMat);

    gl.bindVertexArray(vao);
    // drawElements uses the bound index buffer (IBO in the VAO)
    gl.drawElements(gl.TRIANGLES, mesh.count, gl.UNSIGNED_SHORT, 0);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
