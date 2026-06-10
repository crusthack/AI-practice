'use strict';

// Minimal linear algebra for WebGL2.
// All matrices are column-major Float32Array to match GLSL mat4 memory layout.
// Pass any matrix directly to gl.uniformMatrix4fv(loc, false, mat).

const Vec3 = {
    create:    (x = 0, y = 0, z = 0) => new Float32Array([x, y, z]),
    fromArray: (a) => new Float32Array([a[0], a[1], a[2]]),
    clone:     (a) => new Float32Array([a[0], a[1], a[2]]),
    add:       (a, b) => new Float32Array([a[0]+b[0], a[1]+b[1], a[2]+b[2]]),
    sub:       (a, b) => new Float32Array([a[0]-b[0], a[1]-b[1], a[2]-b[2]]),
    scale:     (a, s) => new Float32Array([a[0]*s, a[1]*s, a[2]*s]),
    dot:       (a, b) => a[0]*b[0] + a[1]*b[1] + a[2]*b[2],
    cross: (a, b) => new Float32Array([
        a[1]*b[2] - a[2]*b[1],
        a[2]*b[0] - a[0]*b[2],
        a[0]*b[1] - a[1]*b[0],
    ]),
    length: (a) => Math.sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]),
    normalize(a) {
        const len = Vec3.length(a);
        return len > 1e-10 ? Vec3.scale(a, 1 / len) : Vec3.create();
    },
    negate: (a) => Vec3.scale(a, -1),
    lerp: (a, b, t) => Vec3.add(Vec3.scale(a, 1 - t), Vec3.scale(b, t)),
};

const Mat4 = {
    // Identity matrix
    identity: () => new Float32Array([
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    ]),

    // Column-major multiply: result = a * b
    multiply(a, b) {
        const out = new Float32Array(16);
        for (let col = 0; col < 4; col++) {
            for (let row = 0; row < 4; row++) {
                let s = 0;
                for (let k = 0; k < 4; k++) s += a[row + k * 4] * b[k + col * 4];
                out[row + col * 4] = s;
            }
        }
        return out;
    },

    // Translation matrix
    translation: (tx, ty, tz) => new Float32Array([
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        tx, ty, tz, 1,
    ]),

    // Uniform scale matrix
    scaling: (sx, sy, sz) => new Float32Array([
        sx, 0,  0,  0,
        0,  sy, 0,  0,
        0,  0,  sz, 0,
        0,  0,  0,  1,
    ]),

    // Rotation around X axis
    rotationX: (rad) => {
        const c = Math.cos(rad), s = Math.sin(rad);
        return new Float32Array([
            1,  0,  0,  0,
            0,  c,  s,  0,
            0, -s,  c,  0,
            0,  0,  0,  1,
        ]);
    },

    // Rotation around Y axis
    rotationY: (rad) => {
        const c = Math.cos(rad), s = Math.sin(rad);
        return new Float32Array([
            c,  0, -s,  0,
            0,  1,  0,  0,
            s,  0,  c,  0,
            0,  0,  0,  1,
        ]);
    },

    // Rotation around Z axis
    rotationZ: (rad) => {
        const c = Math.cos(rad), s = Math.sin(rad);
        return new Float32Array([
             c,  s,  0,  0,
            -s,  c,  0,  0,
             0,  0,  1,  0,
             0,  0,  0,  1,
        ]);
    },

    // Perspective projection (column-major, right-handed, -1..1 depth clip)
    perspective(fovY, aspect, near, far) {
        const f  = 1.0 / Math.tan(fovY * 0.5);
        const nf = 1.0 / (near - far);
        return new Float32Array([
            f / aspect, 0,                    0,  0,
            0,          f,                    0,  0,
            0,          0,  (far + near) * nf, -1,
            0,          0, 2 * far * near * nf,  0,
        ]);
    },

    // Look-at view matrix
    lookAt(eye, center, up) {
        const f = Vec3.normalize(Vec3.sub(center, eye));
        const r = Vec3.normalize(Vec3.cross(f, up));
        const u = Vec3.cross(r, f);
        return new Float32Array([
            r[0],  u[0], -f[0],  0,
            r[1],  u[1], -f[1],  0,
            r[2],  u[2], -f[2],  0,
            -Vec3.dot(r, eye), -Vec3.dot(u, eye), Vec3.dot(f, eye), 1,
        ]);
    },

    // Shorthand: translate an existing matrix
    translate: (m, tx, ty, tz) => Mat4.multiply(m, Mat4.translation(tx, ty, tz)),
    rotateX:   (m, rad)        => Mat4.multiply(m, Mat4.rotationX(rad)),
    rotateY:   (m, rad)        => Mat4.multiply(m, Mat4.rotationY(rad)),
    rotateZ:   (m, rad)        => Mat4.multiply(m, Mat4.rotationZ(rad)),
    scale:     (m, sx, sy, sz) => Mat4.multiply(m, Mat4.scaling(sx, sy, sz)),

    // TRS (compose translate × rotY × rotX × rotZ × scale in one call)
    trs(tx, ty, tz, rx, ry, rz, sx = 1, sy = 1, sz = 1) {
        let m = Mat4.translation(tx, ty, tz);
        m = Mat4.multiply(m, Mat4.rotationY(ry));
        m = Mat4.multiply(m, Mat4.rotationX(rx));
        m = Mat4.multiply(m, Mat4.rotationZ(rz));
        m = Mat4.multiply(m, Mat4.scaling(sx, sy, sz));
        return m;
    },

    // Invert a 4×4 matrix (general case)
    invert(m) {
        const out = new Float32Array(16);
        const [m00, m01, m02, m03, m10, m11, m12, m13,
               m20, m21, m22, m23, m30, m31, m32, m33] = m;
        const b00 = m00*m11 - m01*m10, b01 = m00*m12 - m02*m10;
        const b02 = m00*m13 - m03*m10, b03 = m01*m12 - m02*m11;
        const b04 = m01*m13 - m03*m11, b05 = m02*m13 - m03*m12;
        const b06 = m20*m31 - m21*m30, b07 = m20*m32 - m22*m30;
        const b08 = m20*m33 - m23*m30, b09 = m21*m32 - m22*m31;
        const b10 = m21*m33 - m23*m31, b11 = m22*m33 - m23*m32;
        const det = b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06;
        if (!det) return null;
        const inv = 1 / det;
        out[0]  = (m11*b11 - m12*b10 + m13*b09) * inv;
        out[1]  = (m02*b10 - m01*b11 - m03*b09) * inv;
        out[2]  = (m31*b05 - m32*b04 + m33*b03) * inv;
        out[3]  = (m22*b04 - m21*b05 - m23*b03) * inv;
        out[4]  = (m12*b08 - m10*b11 - m13*b07) * inv;
        out[5]  = (m00*b11 - m02*b08 + m03*b07) * inv;
        out[6]  = (m32*b02 - m30*b05 - m33*b01) * inv;
        out[7]  = (m20*b05 - m22*b02 + m23*b01) * inv;
        out[8]  = (m10*b10 - m11*b08 + m13*b06) * inv;
        out[9]  = (m01*b08 - m00*b10 - m03*b06) * inv;
        out[10] = (m30*b04 - m31*b02 + m33*b00) * inv;
        out[11] = (m21*b02 - m20*b04 - m23*b00) * inv;
        out[12] = (m11*b07 - m10*b09 - m12*b06) * inv;
        out[13] = (m00*b09 - m01*b07 + m02*b06) * inv;
        out[14] = (m31*b01 - m30*b03 - m32*b00) * inv;
        out[15] = (m20*b03 - m21*b01 + m22*b00) * inv;
        return out;
    },

    // Transpose (useful for computing normal matrix)
    transpose(m) {
        return new Float32Array([
            m[0], m[4], m[8],  m[12],
            m[1], m[5], m[9],  m[13],
            m[2], m[6], m[10], m[14],
            m[3], m[7], m[11], m[15],
        ]);
    },

    // Normal matrix = transpose(inverse(modelView))  — returns mat3 as Float32Array[9]
    normalMatrix(modelView) {
        const inv = Mat4.invert(modelView);
        if (!inv) return new Float32Array([1,0,0, 0,1,0, 0,0,1]);
        return new Float32Array([
            inv[0], inv[4], inv[8],
            inv[1], inv[5], inv[9],
            inv[2], inv[6], inv[10],
        ]);
    },
};
