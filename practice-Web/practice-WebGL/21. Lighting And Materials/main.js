'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement Blinn-Phong shading with material properties
// Blinn-Phong is the classic real-time lighting model:
//   finalColor = ambient + diffuse + specular
//   ambient  = Ka * lightColor
//   diffuse  = Kd * lightColor * max(dot(N, L), 0)
//   specular = Ks * lightColor * max(dot(N, H), 0)^shininess
// where H = normalize(L + V) is the half-vector (Blinn's improvement over Phong).
//
// Key steps:
//   1. Upload per-vertex normals; compute the normal matrix in JS:
//      normalMatrix = transpose(inverse(mat3(modelView))).
//      This correctly handles non-uniform scaling.
//   2. In the fragment shader compute N·L (diffuse) and N·H (specular).
//      All vectors must be in the SAME space (view space is conventional).
//   3. Add a second colored light to see how multiple lights compose.

// ----- Shaders -----
const VS = `#version 300 es
layout(location=0) in vec3 a_pos;
layout(location=1) in vec3 a_normal;
uniform mat4 u_mv;        // model-view (world → view space)
uniform mat4 u_proj;
uniform mat3 u_normalMat; // transpose(inverse(mat3(u_mv)))
out vec3 v_posView;    // position in view space
out vec3 v_normalView; // normal in view space
void main() {
    vec4 posView  = u_mv * vec4(a_pos, 1.0);
    v_posView     = posView.xyz;
    // Use the normal matrix, NOT the model matrix, to transform normals.
    // The normal matrix compensates for non-uniform scaling that would
    // otherwise stretch or squash the normals incorrectly.
    v_normalView  = normalize(u_normalMat * a_normal);
    gl_Position   = u_proj * posView;
}`;

const FS = `#version 300 es
precision mediump float;
in vec3 v_posView;
in vec3 v_normalView;
out vec4 outColor;

// Material properties
uniform vec3  u_matAmbient;   // Ka
uniform vec3  u_matDiffuse;   // Kd
uniform vec3  u_matSpecular;  // Ks
uniform float u_matShininess; // exponent

// Two light positions in VIEW space
uniform vec3 u_light0Pos;
uniform vec3 u_light0Color;
uniform vec3 u_light1Pos;
uniform vec3 u_light1Color;

// Compute Blinn-Phong contribution from one light
vec3 blinnPhong(vec3 N, vec3 V, vec3 lightPos, vec3 lightColor) {
    vec3 L = normalize(lightPos - v_posView);  // light direction
    vec3 H = normalize(L + V);                 // half-vector

    float diff = max(dot(N, L), 0.0);
    // Blinn's key insight: replace reflect(−L, N) with the half-vector H.
    // N·H gives the same visual result as N·R but is cheaper and avoids
    // the discontinuity in Phong when the angle exceeds 90°.
    float spec = pow(max(dot(N, H), 0.0), u_matShininess);

    return lightColor * (u_matDiffuse * diff + u_matSpecular * spec);
}

void main() {
    vec3 N = normalize(v_normalView);
    vec3 V = normalize(-v_posView);  // view direction (toward camera at origin)

    vec3 color = u_matAmbient * (u_light0Color + u_light1Color) * 0.1;
    color += blinnPhong(N, V, u_light0Pos, u_light0Color);
    color += blinnPhong(N, V, u_light1Pos, u_light1Color);

    outColor = vec4(color, 1.0);
}`;

function compileShader(type, src) {
    const s = gl.createShader(type);
    gl.shaderSource(s, src); gl.compileShader(s);
    if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(s));
    return s;
}
const prog = gl.createProgram();
gl.attachShader(prog, compileShader(gl.VERTEX_SHADER, VS));
gl.attachShader(prog, compileShader(gl.FRAGMENT_SHADER, FS));
gl.linkProgram(prog);

// ----- Uniform locations -----
const uMV          = gl.getUniformLocation(prog, 'u_mv');
const uProj        = gl.getUniformLocation(prog, 'u_proj');
const uNormalMat   = gl.getUniformLocation(prog, 'u_normalMat');
const uAmbient     = gl.getUniformLocation(prog, 'u_matAmbient');
const uDiffuse     = gl.getUniformLocation(prog, 'u_matDiffuse');
const uSpecular    = gl.getUniformLocation(prog, 'u_matSpecular');
const uShininess   = gl.getUniformLocation(prog, 'u_matShininess');
const uLight0Pos   = gl.getUniformLocation(prog, 'u_light0Pos');
const uLight0Color = gl.getUniformLocation(prog, 'u_light0Color');
const uLight1Pos   = gl.getUniformLocation(prog, 'u_light1Pos');
const uLight1Color = gl.getUniformLocation(prog, 'u_light1Color');

// ----- Build sphere with normals -----
function buildSphere(stacks, slices) {
    const v=[], idx=[];
    for(let i=0;i<=stacks;i++){
        const phi=Math.PI*i/stacks,y=Math.cos(phi),r=Math.sin(phi);
        for(let j=0;j<=slices;j++){
            const t=2*Math.PI*j/slices,x=r*Math.cos(t),z=r*Math.sin(t);
            v.push(x,y,z, x,y,z);  // position + normal (same for unit sphere)
        }
    }
    for(let i=0;i<stacks;i++) for(let j=0;j<slices;j++){
        const a=i*(slices+1)+j,b=a+(slices+1);
        idx.push(a,b,a+1,b,b+1,a+1);
    }
    return {verts:new Float32Array(v),idx:new Uint16Array(idx),count:idx.length};
}
const mesh = buildSphere(32, 32);
const vao = gl.createVertexArray(); gl.bindVertexArray(vao);
const vbo = gl.createBuffer(); gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
gl.bufferData(gl.ARRAY_BUFFER, mesh.verts, gl.STATIC_DRAW);
const STRIDE = 6*4;
gl.enableVertexAttribArray(0); gl.vertexAttribPointer(0,3,gl.FLOAT,false,STRIDE,0);
gl.enableVertexAttribArray(1); gl.vertexAttribPointer(1,3,gl.FLOAT,false,STRIDE,12);
const ibo = gl.createBuffer(); gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, mesh.idx, gl.STATIC_DRAW);

// ----- Matrix helpers -----
function mul(a,b){const o=new Float32Array(16);for(let i=0;i<4;i++)for(let j=0;j<4;j++)for(let k=0;k<4;k++)o[i*4+j]+=a[i*4+k]*b[k*4+j];return o;}
function rotY(t){const c=Math.cos(t),s=Math.sin(t);return new Float32Array([c,0,-s,0,0,1,0,0,s,0,c,0,0,0,0,1]);}
function translate(x,y,z){return new Float32Array([1,0,0,0,0,1,0,0,0,0,1,0,x,y,z,1]);}
function perspective(fov,asp,n,f){const t=Math.tan(fov*Math.PI/360);return new Float32Array([1/(asp*t),0,0,0,0,1/t,0,0,0,0,(f+n)/(n-f),-1,0,0,(2*f*n)/(n-f),0]);}

// TODO Step 1: Normal matrix = transpose(inverse(mat3(MV)))
// For a rigid body (rotation+translation only) the normal matrix equals the
// upper-left 3×3 of MV.  This shortcut breaks for non-uniform scale.
function normalMatrix3(mv) {
    // Extract upper-left 3×3 from column-major mat4
    // For production: compute the actual inverse-transpose.
    return new Float32Array([mv[0],mv[1],mv[2], mv[4],mv[5],mv[6], mv[8],mv[9],mv[10]]);
}

const proj = perspective(45, canvas.width/canvas.height, 0.1, 100);
const cam  = translate(0, 0, -4);

function frame(now) {
    const t = now * 0.001;
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);

    const model = rotY(t * 0.8);
    const mv    = mul(cam, model);

    // TODO Step 2: Light positions in VIEW space (multiply by view matrix)
    // Light positions are defined in WORLD space, then transformed to view
    // space so all lighting calculations happen in a consistent coordinate system.
    const lp0w = [3*Math.cos(t), 2, 3*Math.sin(t), 1];
    const lp1w = [-3, 1, 0, 1];
    function transformPoint(m, p) {
        return [
            m[0]*p[0]+m[4]*p[1]+m[8]*p[2]+m[12],
            m[1]*p[0]+m[5]*p[1]+m[9]*p[2]+m[13],
            m[2]*p[0]+m[6]*p[1]+m[10]*p[2]+m[14]
        ];
    }
    const lp0v = transformPoint(cam, lp0w);
    const lp1v = transformPoint(cam, lp1w);

    gl.useProgram(prog);
    gl.uniformMatrix4fv(uMV,        false, mv);
    gl.uniformMatrix4fv(uProj,      false, proj);
    gl.uniformMatrix3fv(uNormalMat, false, normalMatrix3(mv));

    // TODO Step 3: Set material and light uniforms
    gl.uniform3fv(uAmbient,     [0.1, 0.1, 0.15]);
    gl.uniform3fv(uDiffuse,     [0.3, 0.5, 0.9]);
    gl.uniform3fv(uSpecular,    [1.0, 1.0, 1.0]);
    gl.uniform1f(uShininess,    64.0);
    gl.uniform3fv(uLight0Pos,   lp0v);
    gl.uniform3fv(uLight0Color, [1.0, 0.9, 0.7]);
    gl.uniform3fv(uLight1Pos,   lp1v);
    gl.uniform3fv(uLight1Color, [0.2, 0.4, 1.0]);

    gl.bindVertexArray(vao);
    gl.drawElements(gl.TRIANGLES, mesh.count, gl.UNSIGNED_SHORT, 0);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
