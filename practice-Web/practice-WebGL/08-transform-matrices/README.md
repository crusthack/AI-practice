# 08 - Transform Matrices

## Goal

Demonstrate the full MVP (Model-View-Projection) pipeline by spinning a 3D colored cube seen from a perspective camera, uploading a single combined `mat4` uniform each frame.

## Key API Calls / Concepts

| Item | Description |
|---|---|
| `gl.uniformMatrix4fv(loc, false, mat4)` | Uploads a column-major 4x4 matrix; the `false` means no transpose |
| `Mat4.perspective(fov, aspect, near, far)` | Constructs a frustum projection matrix — maps 3D depth to clip-space Z |
| `Mat4.lookAt(eye, center, up)` | Constructs a view matrix that orients the camera in world space |
| `Mat4.rotateY(angle)` | Constructs a model matrix rotating around the Y axis by `angle` radians |
| MVP multiplication order | `proj * view * model` — rightmost applied first; column-major WebGL convention |

## Implementation Steps

1. Build `proj` and `view` once outside the loop (they do not change per frame).
2. Each frame recompute `model = Mat4.rotateY(t)` where `t` is elapsed seconds.
3. Multiply `mvp = proj * view * model`, upload with `gl.uniformMatrix4fv`, then draw the indexed cube mesh.

## Expected Output

A rainbow-colored 3D cube spinning continuously around the Y axis, rendered with correct perspective foreshortening, proving the MVP pipeline is working end-to-end.
