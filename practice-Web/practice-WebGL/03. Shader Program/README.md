# 03 - Shader Program

## Goal

Compile GLSL ES vertex and fragment shaders with explicit error reporting, link a program, cache uniform locations, and use per-frame uniforms to animate color.

## Key API Calls

| Call | Purpose |
|---|---|
| `gl.getShaderParameter(s, COMPILE_STATUS)` | Check if compilation succeeded |
| `gl.getShaderInfoLog(shader)` | Retrieve compile error message |
| `gl.getProgramParameter(p, LINK_STATUS)` | Check if linking succeeded |
| `gl.getProgramInfoLog(prog)` | Retrieve link error message |
| `gl.validateProgram(prog)` | Verify program works in the current GL state |
| `gl.detachShader / deleteShader` | Release shader objects after linking |
| `gl.getUniformLocation(prog, name)` | Query a uniform's binding point |
| `gl.uniform1f / uniform2f / uniform4f` | Upload scalar/vector uniforms |

## Shader Lifecycle

```
createShader → shaderSource → compileShader → [check] →
createProgram → attachShader × 2 → linkProgram → [check] →
detachShader + deleteShader × 2 (memory freed on GPU)
```

## Uniform Workflow

1. **Compile time**: declare `uniform float u_time;` in GLSL.
2. **After link**: `gl.getUniformLocation(prog, 'u_time')` — returns null if unused/optimized away.
3. **Per frame**: `gl.uniform1f(loc, value)` — program must be bound with `useProgram` first.

## Expected Output

A triangle that pulses between blue and white, with positions specified in pixel space and transformed to NDC inside the vertex shader.
