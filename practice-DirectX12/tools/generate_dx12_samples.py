from __future__ import annotations

import uuid
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOLUTION_GUID_CPP = "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"
GUID_NAMESPACE = uuid.UUID("54f655bb-4590-4b51-9c60-f6ce0e0c2ab1")


SAMPLES = [
    {
        "number": "01",
        "name": "Dx12Basic",
        "goal": "Create a Win32 window, initialize the DX12 device/swap chain/command queue, clear a back buffer, and present.",
        "status": "Implemented",
        "new_concepts": [
            "Win32 window creation for a graphics sample",
            "DXGI factory and hardware adapter selection",
            "DirectX 12 device and direct command queue",
            "Flip-model swap chain with two back buffers",
            "RTV descriptor heap and render target views",
            "Command allocator, command list, resource barriers, fence, and present",
        ],
        "api_calls": [
            "CreateDXGIFactory2",
            "IDXGIFactory4::EnumAdapters1",
            "D3D12CreateDevice",
            "ID3D12Device::CreateCommandQueue",
            "IDXGIFactory4::CreateSwapChainForHwnd",
            "ID3D12Device::CreateDescriptorHeap",
            "ID3D12Device::CreateRenderTargetView",
            "ID3D12Device::CreateCommandAllocator",
            "ID3D12Device::CreateCommandList",
            "ID3D12GraphicsCommandList::ResourceBarrier",
            "ID3D12GraphicsCommandList::ClearRenderTargetView",
            "ID3D12CommandQueue::ExecuteCommandLists",
            "IDXGISwapChain::Present",
            "ID3D12Device::CreateFence",
        ],
        "expected": "A 1280x720 window titled `01. Dx12Basic` opens and shows a solid dark blue-gray background.",
        "color": (0.08, 0.13, 0.20),
        "shader_policy": "placeholder shader file; no shader compilation",
        "asset_policy": "no external assets",
    },
    {
        "number": "02",
        "name": "HelloTriangle",
        "goal": "Build the first graphics pipeline and draw a triangle with external HLSL shaders.",
        "status": "Scaffold",
        "new_concepts": ["root signature", "graphics pipeline state", "viewport and scissor", "external HLSL shaders", "non-indexed draw call"],
        "api_calls": ["D3D12SerializeRootSignature", "ID3D12Device::CreateRootSignature", "D3DCompileFromFile", "ID3D12Device::CreateGraphicsPipelineState", "ID3D12GraphicsCommandList::DrawInstanced"],
        "expected": "Scaffold visual: a smoke-test window clears to a deep blue background. Final target: one colored triangle.",
        "color": (0.05, 0.10, 0.22),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural triangle data planned",
    },
    {
        "number": "03",
        "name": "Shaders",
        "goal": "Separate vertex and pixel shader responsibilities and pass interpolated color to the pixel shader.",
        "status": "Scaffold",
        "new_concepts": ["shader entry points", "input/output semantics", "interpolators", "shader compilation diagnostics"],
        "api_calls": ["D3DCompileFromFile", "ID3DBlob::GetBufferPointer", "D3D12_GRAPHICS_PIPELINE_STATE_DESC", "ID3D12Device::CreateGraphicsPipelineState"],
        "expected": "Scaffold visual: a smoke-test window clears to a muted indigo background. Final target: shader-driven colored geometry.",
        "color": (0.10, 0.08, 0.24),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural geometry planned",
    },
    {
        "number": "04",
        "name": "Index Buffer",
        "goal": "Draw shared-vertex geometry through an index buffer.",
        "status": "Scaffold",
        "new_concepts": ["index buffer", "index buffer view", "shared vertices", "indexed draw call"],
        "api_calls": ["ID3D12Device::CreateCommittedResource", "ID3D12GraphicsCommandList::IASetIndexBuffer", "ID3D12GraphicsCommandList::DrawIndexedInstanced"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark teal background. Final target: indexed rectangle or quad.",
        "color": (0.04, 0.16, 0.16),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural vertex and index data planned",
    },
    {
        "number": "05",
        "name": "Constant Buffer",
        "goal": "Pass per-frame CPU data to shaders through a constant buffer.",
        "status": "Scaffold",
        "new_concepts": ["constant buffer", "CBV descriptor", "256-byte alignment", "mapped upload heap"],
        "api_calls": ["ID3D12Device::CreateCommittedResource", "ID3D12Device::CreateConstantBufferView", "ID3D12Resource::Map", "ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark green background. Final target: animated color or transform controlled by a constant buffer.",
        "color": (0.06, 0.18, 0.11),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural data planned",
    },
    {
        "number": "06",
        "name": "Render Pipeline",
        "goal": "Inspect the full input assembler through output merger path for a basic draw.",
        "status": "Scaffold",
        "new_concepts": ["input layout", "primitive topology", "rasterizer state", "blend state", "output merger"],
        "api_calls": ["D3D12_INPUT_ELEMENT_DESC", "IASetPrimitiveTopology", "RSSetViewports", "RSSetScissorRects", "OMSetRenderTargets"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark slate background. Final target: annotated basic pipeline draw.",
        "color": (0.12, 0.14, 0.17),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural geometry planned",
    },
    {
        "number": "07",
        "name": "Coordinate Spaces",
        "goal": "Show how local, world, view, projection, NDC, and screen spaces relate.",
        "status": "Scaffold",
        "new_concepts": ["local space", "world space", "view space", "projection space", "NDC"],
        "api_calls": ["DirectX::XMMatrixIdentity", "DirectX::XMMatrixLookAtLH", "DirectX::XMMatrixPerspectiveFovLH", "ID3D12Resource::Map"],
        "expected": "Scaffold visual: a smoke-test window clears to a desaturated blue-green background. Final target: transformed geometry with coordinate-space controls.",
        "color": (0.07, 0.15, 0.18),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural geometry planned",
    },
    {
        "number": "08",
        "name": "Matrix",
        "goal": "Animate geometry with translation, rotation, and scale matrices.",
        "status": "Scaffold",
        "new_concepts": ["translation matrix", "rotation matrix", "scale matrix", "matrix multiplication order"],
        "api_calls": ["DirectX::XMMatrixTranslation", "DirectX::XMMatrixRotationY", "DirectX::XMMatrixScaling", "DirectX::XMStoreFloat4x4"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark violet-gray background. Final target: animated transformed geometry.",
        "color": (0.15, 0.11, 0.18),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural geometry planned",
    },
    {
        "number": "09",
        "name": "Camera",
        "goal": "Move a view camera with keyboard and mouse input.",
        "status": "Scaffold",
        "new_concepts": ["camera position", "camera basis vectors", "view matrix", "keyboard input", "mouse look"],
        "api_calls": ["GetAsyncKeyState", "DirectX::XMMatrixLookToLH", "DirectX::XMVector3Normalize", "ID3D12Resource::Map"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark navy background. Final target: camera-controlled scene.",
        "color": (0.04, 0.07, 0.16),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural geometry planned",
        "controls": "Planned: WASD movement and mouse look. Scaffold: Esc exits.",
    },
    {
        "number": "10",
        "name": "3D Cube",
        "goal": "Render a rotating 3D cube with vertex, index, constant, and depth resources.",
        "status": "Scaffold",
        "new_concepts": ["cube mesh", "depth testing", "perspective projection", "back-face visibility"],
        "api_calls": ["CreateCommittedResource", "CreateDepthStencilView", "ClearDepthStencilView", "DrawIndexedInstanced"],
        "expected": "Scaffold visual: a smoke-test window clears to a charcoal background. Final target: rotating 3D cube.",
        "color": (0.10, 0.10, 0.12),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural cube data planned",
    },
    {
        "number": "11",
        "name": "Depth Buffer",
        "goal": "Create and use a depth buffer so nearer fragments occlude farther fragments.",
        "status": "Scaffold",
        "new_concepts": ["depth resource", "DSV heap", "depth clear", "depth comparison", "depth/stencil state"],
        "api_calls": ["ID3D12Device::CreateDescriptorHeap", "ID3D12Device::CreateDepthStencilView", "ClearDepthStencilView", "D3D12_DEPTH_STENCIL_DESC"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark blue-black background. Final target: overlapping 3D geometry with correct occlusion.",
        "color": (0.03, 0.05, 0.09),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural geometry planned",
    },
    {
        "number": "12",
        "name": "Texture",
        "goal": "Upload a texture and sample it through an SRV descriptor in the pixel shader.",
        "status": "Scaffold",
        "new_concepts": ["texture resource", "SRV descriptor", "sampler", "subresource upload", "UV coordinates"],
        "api_calls": ["CreateCommittedResource", "UpdateSubresources", "CreateShaderResourceView", "CreateSampler", "SetDescriptorHeaps"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark forest background. Final target: textured geometry.",
        "color": (0.03, 0.12, 0.08),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "procedural checker texture planned",
    },
    {
        "number": "13",
        "name": "Lighting",
        "goal": "Compute simple diffuse lighting from normals and a directional light.",
        "status": "Scaffold",
        "new_concepts": ["normal vectors", "directional light", "Lambert diffuse", "per-pixel lighting"],
        "api_calls": ["D3D12_INPUT_ELEMENT_DESC", "D3DCompileFromFile", "CreateGraphicsPipelineState", "SetGraphicsRootDescriptorTable"],
        "expected": "Scaffold visual: a smoke-test window clears to a dim olive background. Final target: lit 3D object.",
        "color": (0.11, 0.13, 0.07),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural geometry planned",
    },
    {
        "number": "14",
        "name": "Material",
        "goal": "Separate material parameters from light parameters and bind them per object.",
        "status": "Scaffold",
        "new_concepts": ["material constants", "albedo", "specular controls", "per-object binding"],
        "api_calls": ["CreateConstantBufferView", "SetGraphicsRootDescriptorTable", "DrawIndexedInstanced"],
        "expected": "Scaffold visual: a smoke-test window clears to a subdued purple-gray background. Final target: multiple materials under the same light.",
        "color": (0.14, 0.10, 0.15),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural material data planned",
    },
    {
        "number": "15",
        "name": "Rasterizer State",
        "goal": "Change fill and culling behavior through rasterizer state in the PSO.",
        "status": "Scaffold",
        "new_concepts": ["wireframe fill", "solid fill", "front/back culling", "rasterizer desc"],
        "api_calls": ["D3D12_RASTERIZER_DESC", "D3D12_CULL_MODE", "D3D12_FILL_MODE", "CreateGraphicsPipelineState"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark steel background. Final target: toggleable wireframe and culling.",
        "color": (0.09, 0.12, 0.14),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural geometry planned",
        "controls": "Planned: key toggles for fill and cull modes. Scaffold: Esc exits.",
    },
    {
        "number": "16",
        "name": "Blend State",
        "goal": "Enable alpha blending and render transparent geometry in a controlled order.",
        "status": "Scaffold",
        "new_concepts": ["blend state", "alpha blending", "render ordering", "source/destination factors"],
        "api_calls": ["D3D12_BLEND_DESC", "D3D12_RENDER_TARGET_BLEND_DESC", "CreateGraphicsPipelineState", "DrawInstanced"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark wine background. Final target: overlapping transparent primitives.",
        "color": (0.16, 0.07, 0.10),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural geometry planned",
    },
    {
        "number": "17",
        "name": "Model Loading",
        "goal": "Load mesh data from an external file and upload it into GPU buffers.",
        "status": "Scaffold",
        "new_concepts": ["mesh file parsing", "external asset copy", "vertex/index upload", "model bounds"],
        "api_calls": ["CreateCommittedResource", "UpdateSubresources", "IASetVertexBuffers", "IASetIndexBuffer", "DrawIndexedInstanced"],
        "expected": "Scaffold visual: a smoke-test window clears to a neutral dark gray background. Final target: imported model rendered on screen.",
        "color": (0.11, 0.11, 0.11),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "external mesh asset planned",
    },
    {
        "number": "18",
        "name": "Scene",
        "goal": "Render multiple scene objects with separate transforms, materials, and draw calls.",
        "status": "Scaffold",
        "new_concepts": ["scene object list", "per-object constants", "multiple draw calls", "simple update loop"],
        "api_calls": ["SetGraphicsRootDescriptorTable", "IASetVertexBuffers", "IASetIndexBuffer", "DrawIndexedInstanced"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark cyan-gray background. Final target: multi-object scene.",
        "color": (0.07, 0.13, 0.14),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "procedural and/or copied assets planned",
    },
    {
        "number": "19",
        "name": "Instancing",
        "goal": "Draw many copies of the same mesh with per-instance data.",
        "status": "Scaffold",
        "new_concepts": ["instance buffer", "per-instance input layout", "instance count", "draw call reduction"],
        "api_calls": ["IASetVertexBuffers", "DrawInstanced", "DrawIndexedInstanced", "D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark moss background. Final target: many instanced objects.",
        "color": (0.07, 0.12, 0.06),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "inline/procedural instance data planned",
    },
    {
        "number": "20",
        "name": "Render To Texture",
        "goal": "Render a scene into an offscreen texture before presenting it.",
        "status": "Scaffold",
        "new_concepts": ["offscreen render target", "RTV and SRV for one texture", "render pass separation", "resource state transitions"],
        "api_calls": ["CreateRenderTargetView", "CreateShaderResourceView", "ResourceBarrier", "OMSetRenderTargets"],
        "expected": "Scaffold visual: a smoke-test window clears to a deep teal background. Final target: offscreen texture displayed on screen.",
        "color": (0.02, 0.13, 0.13),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "procedural scene planned",
    },
    {
        "number": "21",
        "name": "Post Processing",
        "goal": "Apply a full-screen post-processing pass to an offscreen render target.",
        "status": "Scaffold",
        "new_concepts": ["full-screen triangle", "post-process pixel shader", "SRV input texture", "two-pass rendering"],
        "api_calls": ["CreateShaderResourceView", "SetDescriptorHeaps", "DrawInstanced", "ResourceBarrier"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark magenta-gray background. Final target: grayscale or color-shift post effect.",
        "color": (0.13, 0.08, 0.12),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "procedural scene planned",
    },
    {
        "number": "22",
        "name": "Shadow Mapping",
        "goal": "Render depth from a light view and sample it to shade shadows.",
        "status": "Scaffold",
        "new_concepts": ["shadow map", "light view projection", "depth-only pass", "shadow comparison"],
        "api_calls": ["CreateDepthStencilView", "CreateShaderResourceView", "ClearDepthStencilView", "DrawIndexedInstanced"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark brown-gray background. Final target: object casting a shadow.",
        "color": (0.13, 0.10, 0.08),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "procedural scene planned",
    },
    {
        "number": "23",
        "name": "Skybox",
        "goal": "Render a cubemap skybox behind the scene.",
        "status": "Scaffold",
        "new_concepts": ["cubemap texture", "skybox cube", "depth behavior", "sampler state"],
        "api_calls": ["CreateShaderResourceView", "CreateSampler", "DrawIndexedInstanced", "ResourceBarrier"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark horizon-blue background. Final target: cubemap skybox.",
        "color": (0.04, 0.09, 0.14),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "procedural or generated cubemap planned",
    },
    {
        "number": "24",
        "name": "Deferred Rendering",
        "goal": "Write geometry data into a G-buffer and light it in a second pass.",
        "status": "Scaffold",
        "new_concepts": ["G-buffer", "multiple render targets", "lighting pass", "full-screen pass"],
        "api_calls": ["OMSetRenderTargets", "CreateRenderTargetView", "CreateShaderResourceView", "DrawInstanced"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark graphite background. Final target: deferred shaded scene.",
        "color": (0.08, 0.09, 0.10),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "procedural scene planned",
    },
    {
        "number": "25",
        "name": "ImGui Integration",
        "goal": "Integrate Dear ImGui as a debug UI overlay for DX12 samples.",
        "status": "Scaffold",
        "new_concepts": ["ImGui context", "descriptor heap for UI", "frame UI build", "overlay rendering"],
        "api_calls": ["ImGui::CreateContext", "ImGui_ImplWin32_Init", "ImGui_ImplDX12_Init", "ImGui_ImplDX12_RenderDrawData"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark neutral background. Final target: DX12 scene with ImGui overlay.",
        "color": (0.09, 0.09, 0.10),
        "shader_policy": "Dear ImGui shaders handled by backend in final version",
        "asset_policy": "external Dear ImGui source planned",
    },
    {
        "number": "26",
        "name": "Mini Engine",
        "goal": "Extract repeated DX12 setup into a small learning-oriented engine layer.",
        "status": "Scaffold",
        "new_concepts": ["device wrapper", "frame resources", "resource lifetime", "sample app interface"],
        "api_calls": ["CreateCommandQueue", "CreateCommandAllocator", "CreateCommandList", "CreateFence", "ResourceBarrier"],
        "expected": "Scaffold visual: a smoke-test window clears to a dark blue-gray background. Final target: same visual built through a mini engine layer.",
        "color": (0.07, 0.10, 0.13),
        "shader_policy": "external HLSL source planned where needed",
        "asset_policy": "procedural assets planned",
    },
    {
        "number": "27",
        "name": "Final Project",
        "goal": "Combine the learning samples into a small interactive rendered scene.",
        "status": "Scaffold",
        "new_concepts": ["scene composition", "camera", "materials", "lighting", "post-processing", "debug UI"],
        "api_calls": ["DrawIndexedInstanced", "SetDescriptorHeaps", "ResourceBarrier", "ExecuteCommandLists", "Present"],
        "expected": "Scaffold visual: a smoke-test window clears to a final-project dark background. Final target: interactive rendered scene.",
        "color": (0.06, 0.08, 0.11),
        "shader_policy": "external HLSL source planned",
        "asset_policy": "procedural and external assets planned",
    },
]


def _dx12_sample(number, name, goal, new_concepts, api_calls, expected, color, shader_policy="external HLSL source planned", asset_policy="inline/procedural data planned", controls=None, status="Scaffold", preserve_existing=False):
    sample = {
        "number": number,
        "name": name,
        "goal": goal,
        "status": status,
        "new_concepts": new_concepts,
        "api_calls": api_calls,
        "expected": expected,
        "color": color,
        "shader_policy": shader_policy,
        "asset_policy": asset_policy,
    }
    if controls:
        sample["controls"] = controls
    if preserve_existing:
        sample["preserve_existing"] = True
    return sample


# DX12-specific sequence. This intentionally replaces the older DX11-shaped list above.
SAMPLES = [
    _dx12_sample(
        "01",
        "Dx12Basic",
        "Create a Win32 window, initialize the DX12 device/swap chain/command queue, clear a back buffer, and present.",
        [
            "Win32 window creation for a graphics sample",
            "DXGI factory and hardware adapter selection",
            "DirectX 12 device and direct command queue",
            "Flip-model swap chain with two back buffers",
            "RTV descriptor heap and render target views",
            "Command allocator, command list, resource barriers, fence, and present",
        ],
        [
            "CreateDXGIFactory2",
            "IDXGIFactory4::EnumAdapters1",
            "D3D12CreateDevice",
            "ID3D12Device::CreateCommandQueue",
            "IDXGIFactory4::CreateSwapChainForHwnd",
            "ID3D12Device::CreateDescriptorHeap",
            "ID3D12Device::CreateRenderTargetView",
            "ID3D12Device::CreateCommandAllocator",
            "ID3D12Device::CreateCommandList",
            "ID3D12GraphicsCommandList::ResourceBarrier",
            "IDXGISwapChain::Present",
            "ID3D12Device::CreateFence",
        ],
        "A 1280x720 window titled `01. Dx12Basic` opens and shows a solid dark blue-gray background.",
        (0.08, 0.13, 0.20),
        shader_policy="placeholder shader file; no shader compilation",
        asset_policy="no external assets",
        status="Implemented",
    ),
    _dx12_sample("02", "HelloTriangle", "Build the first root signature and graphics pipeline state object, then draw a triangle.", ["root signature", "graphics pipeline state object", "viewport and scissor", "external HLSL shaders", "non-indexed draw call"], ["D3D12SerializeRootSignature", "ID3D12Device::CreateRootSignature", "D3DCompileFromFile", "ID3D12Device::CreateGraphicsPipelineState", "ID3D12GraphicsCommandList::DrawInstanced"], "A 1280x720 window titled `02. HelloTriangle` opens, clears to a deep blue background, and renders one colored triangle.", (0.05, 0.10, 0.22), asset_policy="inline/procedural triangle vertices", status="Implemented", preserve_existing=True),
    _dx12_sample("03", "Vertex Buffer Upload", "Upload vertex data into GPU resources and bind a vertex buffer view.", ["upload heap", "default heap", "vertex buffer resource", "vertex buffer view", "copy command"], ["ID3D12Device::CreateCommittedResource", "ID3D12Resource::Map", "ID3D12GraphicsCommandList::CopyBufferRegion", "ID3D12GraphicsCommandList::ResourceBarrier", "ID3D12GraphicsCommandList::IASetVertexBuffers"], "A 1280x720 window titled `03. Vertex Buffer Upload` opens, clears to a muted indigo background, and renders one colored triangle from a default-heap vertex buffer.", (0.10, 0.08, 0.24), asset_policy="inline/procedural vertex data", status="Implemented", preserve_existing=True),
    _dx12_sample("04", "Index Buffer", "Draw shared-vertex geometry through an index buffer.", ["index buffer", "index buffer view", "shared vertices", "indexed draw call"], ["ID3D12Device::CreateCommittedResource", "ID3D12Resource::Map", "ID3D12GraphicsCommandList::IASetVertexBuffers", "ID3D12GraphicsCommandList::IASetIndexBuffer", "ID3D12GraphicsCommandList::DrawIndexedInstanced"], "A 1280x720 window titled `04. Index Buffer` opens, clears to a dark teal background, and renders a colored rectangle using six indices and four shared vertices.", (0.04, 0.16, 0.16), asset_policy="inline/procedural vertex and index data", status="Implemented", preserve_existing=True),
    _dx12_sample("05", "Root Signature And Constants", "Bind small per-draw values through root constants and compare them with constant-buffer binding.", ["root parameters", "root constants", "32-bit root values", "root argument cost", "shader-visible constant layout"], ["D3D12SerializeRootSignature", "ID3D12Device::CreateRootSignature", "ID3D12GraphicsCommandList::SetGraphicsRoot32BitConstants", "ID3D12Device::CreateGraphicsPipelineState", "ID3D12GraphicsCommandList::DrawInstanced"], "A 1280x720 window titled `05. Root Signature And Constants` opens, clears to a dark green background, and renders a colored triangle that shifts and pulses using root constants.", (0.06, 0.18, 0.11), status="Implemented", preserve_existing=True),
    _dx12_sample("06", "Descriptor Heap", "Create shader-visible descriptor heaps and bind CBV descriptors through descriptor tables.", ["CPU descriptor handle", "GPU descriptor handle", "shader-visible heap", "descriptor table", "descriptor increment size"], ["ID3D12Device::CreateDescriptorHeap", "ID3D12Device::CreateConstantBufferView", "ID3D12GraphicsCommandList::SetDescriptorHeaps", "ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable", "ID3D12GraphicsCommandList::DrawInstanced"], "A 1280x720 window titled `06. Descriptor Heap` opens, clears to a dark slate background, and renders a triangle whose offset and tint come from a CBV descriptor table.", (0.12, 0.14, 0.17), status="Implemented", preserve_existing=True),
    _dx12_sample("07", "Texture Upload", "Upload a procedural texture and sample it through an SRV and sampler.", ["texture resource", "subresource footprint", "SRV descriptor", "sampler", "texture state transition"], ["ID3D12Device::GetCopyableFootprints", "ID3D12GraphicsCommandList::CopyTextureRegion", "ID3D12Device::CreateShaderResourceView", "ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable", "ID3D12GraphicsCommandList::ResourceBarrier"], "A 1280x720 window titled `07. Texture Upload` opens, clears to a desaturated blue-green background, and renders a quad textured with a procedural checker pattern.", (0.07, 0.15, 0.18), asset_policy="procedural checker texture", status="Implemented", preserve_existing=True),
    _dx12_sample("08", "Depth Buffer", "Create a depth resource and DSV heap so nearer fragments occlude farther fragments.", ["depth resource", "DSV heap", "depth clear", "depth comparison", "depth/stencil state"], ["ID3D12Device::CreateDescriptorHeap", "ID3D12Device::CreateDepthStencilView", "ID3D12GraphicsCommandList::ClearDepthStencilView", "ID3D12GraphicsCommandList::OMSetRenderTargets", "D3D12_DEPTH_STENCIL_DESC"], "A 1280x720 window titled `08. Depth Buffer` opens, clears to a dark blue-black background, and renders overlapping triangles with depth testing enabled.", (0.03, 0.05, 0.09), status="Implemented", preserve_existing=True),
    _dx12_sample("09", "Transform Matrices", "Pass world, view, and projection matrices to shaders and transform geometry.", ["world matrix", "view matrix", "projection matrix", "matrix upload", "HLSL constant layout"], ["DirectX::XMMatrixRotationZ", "DirectX::XMMatrixLookAtLH", "DirectX::XMMatrixPerspectiveFovLH", "DirectX::XMStoreFloat4x4", "ID3D12Device::CreateConstantBufferView"], "A 1280x720 window titled `09. Transform Matrices` opens, clears to a dark violet-gray background, and renders a rotating transformed quad using a WVP constant buffer.", (0.15, 0.11, 0.18), status="Implemented", preserve_existing=True),
    _dx12_sample("10", "Camera", "Move a view camera with keyboard and mouse input.", ["camera position", "camera basis vectors", "view matrix", "keyboard input", "mouse look"], ["GetAsyncKeyState", "GetCursorPos", "DirectX::XMMatrixLookToLH", "DirectX::XMVector3Normalize", "ID3D12Device::CreateConstantBufferView"], "A 1280x720 window titled `10. Camera` opens, clears to a dark navy background, and renders three colored cubes that can be inspected by moving the camera.", (0.04, 0.07, 0.16), controls="WASD moves horizontally, Q/E moves down/up, Shift increases movement speed, hold the right mouse button and move the mouse to look around, and Esc exits.", status="Implemented", preserve_existing=True),
    _dx12_sample("11", "Frame Resources", "Use per-frame command allocators, constant buffers, and fence values for multiple frames in flight.", ["frames in flight", "per-frame allocator", "per-frame constants", "fence value tracking"], ["ID3D12Device::CreateCommandAllocator", "ID3D12Device::CreateConstantBufferView", "ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable", "ID3D12CommandQueue::Signal", "ID3D12Fence::SetEventOnCompletion", "IDXGISwapChain3::GetCurrentBackBufferIndex"], "A 1280x720 window titled `11. Frame Resources` opens, clears to a charcoal background, and renders a triangle whose tint and offset are updated through the current frame resource.", (0.10, 0.10, 0.12), status="Implemented", preserve_existing=True),
    _dx12_sample("12", "Resource Barriers", "Practice explicit resource state transitions for render targets, copy destinations, and shader resources.", ["transition barrier", "render target state", "copy state", "shader resource state", "texture upload synchronization"], ["ID3D12GraphicsCommandList::ResourceBarrier", "D3D12_RESOURCE_BARRIER", "D3D12_RESOURCE_STATE_COPY_DEST", "D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE", "D3D12_RESOURCE_STATE_RENDER_TARGET"], "A 1280x720 window titled `12. Resource Barriers` opens, clears to a dark forest background, and renders a textured quad whose texture was transitioned from copy destination to pixel shader resource.", (0.03, 0.12, 0.08), status="Implemented", preserve_existing=True),
    _dx12_sample("13", "Upload And Default Heaps", "Separate CPU-visible upload resources from GPU-local default resources and copy between them.", ["upload heap", "default heap", "intermediate resource", "copy queue concept", "GPU-local resource"], ["ID3D12Device::CreateCommittedResource", "D3D12_HEAP_TYPE_UPLOAD", "D3D12_HEAP_TYPE_DEFAULT", "ID3D12GraphicsCommandList::CopyResource", "D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER", "D3D12_RESOURCE_STATE_INDEX_BUFFER"], "A 1280x720 window titled `13. Upload And Default Heaps` opens, clears to a dim olive background, and renders a colored indexed rectangle from default heap vertex and index buffers.", (0.11, 0.13, 0.07), status="Implemented", preserve_existing=True),
    _dx12_sample("14", "Pipeline State Variants", "Create and switch between multiple PSOs for different shaders or render states.", ["PSO cache concept", "rasterizer state variants", "solid fill", "wireframe fill", "state switching"], ["ID3D12Device::CreateGraphicsPipelineState", "ID3D12GraphicsCommandList::SetPipelineState", "D3D12_GRAPHICS_PIPELINE_STATE_DESC", "D3D12_RASTERIZER_DESC"], "A 1280x720 window titled `14. Pipeline State Variants` opens, clears to a subdued purple-gray background, and renders a solid rectangle beside a wireframe rectangle.", (0.14, 0.10, 0.15), controls="Press `Esc` or close the window to exit.", status="Implemented", preserve_existing=True),
    _dx12_sample("15", "Rasterizer State", "Change fill and culling behavior through rasterizer state in the PSO.", ["wireframe fill", "solid fill", "front/back culling", "rasterizer desc"], ["D3D12_RASTERIZER_DESC", "D3D12_CULL_MODE", "D3D12_FILL_MODE", "ID3D12Device::CreateGraphicsPipelineState"], "A 1280x720 window titled `15. Rasterizer State` opens, clears to a dark steel background, and renders solid, wireframe, and back-face-culling examples side by side.", (0.09, 0.12, 0.14), controls="Press `Esc` or close the window to exit.", status="Implemented", preserve_existing=True),
    _dx12_sample("16", "Blend State", "Enable alpha blending and render transparent geometry in a controlled order.", ["blend state", "alpha blending", "render ordering", "source/destination factors"], ["D3D12_BLEND_DESC", "D3D12_RENDER_TARGET_BLEND_DESC", "ID3D12Device::CreateGraphicsPipelineState", "ID3D12GraphicsCommandList::DrawInstanced"], "A 1280x720 window titled `16. Blend State` opens, clears to a dark wine background, and renders two overlapping transparent triangles with visible alpha blending.", (0.16, 0.07, 0.10), status="Implemented", preserve_existing=True),
    _dx12_sample("17", "Render To Texture", "Render into an offscreen texture and then sample it while presenting to the swap chain.", ["offscreen render target", "RTV and SRV for one texture", "render pass separation", "state transition between render target and SRV"], ["ID3D12Device::CreateRenderTargetView", "ID3D12Device::CreateShaderResourceView", "ID3D12GraphicsCommandList::ResourceBarrier", "ID3D12GraphicsCommandList::OMSetRenderTargets"], "A 1280x720 window titled `17. Render To Texture` opens and displays a colored triangle that was first rendered into an offscreen texture and then sampled onto the back buffer.", (0.02, 0.13, 0.13), status="Implemented", preserve_existing=True),
    _dx12_sample("18", "Compute Shader", "Dispatch a compute shader and synchronize its output for rendering or readback.", ["compute root signature", "compute PSO", "dispatch dimensions", "UAV output"], ["ID3D12Device::CreateComputePipelineState", "ID3D12GraphicsCommandList::SetComputeRootSignature", "ID3D12GraphicsCommandList::Dispatch", "ID3D12Device::CreateUnorderedAccessView"], "A 1280x720 window titled `18. Compute Shader` opens and displays an animated compute-generated color pattern sampled from a UAV-written texture.", (0.07, 0.13, 0.14), status="Implemented", preserve_existing=True),
    _dx12_sample("19", "UAV And Readback", "Write GPU data through a UAV and read selected results back to the CPU.", ["unordered access view", "readback heap", "UAV barrier", "GPU-to-CPU copy"], ["ID3D12Device::CreateUnorderedAccessView", "ID3D12GraphicsCommandList::CopyResource", "D3D12_HEAP_TYPE_READBACK", "ID3D12Resource::Map"], "A 1280x720 window titled `19. UAV And Readback` opens and slowly changes its clear color based on a value written by the GPU and read back by the CPU.", (0.07, 0.12, 0.06), shader_policy="external HLSL compute shader", status="Implemented", preserve_existing=True),
    _dx12_sample("20", "Model Loading", "Load mesh data from an external file and upload it into GPU buffers.", ["mesh file parsing", "external asset copy", "vertex/index upload", "model bounds"], ["ID3D12Device::CreateCommittedResource", "ID3D12Resource::Map", "ID3D12GraphicsCommandList::IASetVertexBuffers", "ID3D12GraphicsCommandList::IASetIndexBuffer", "ID3D12GraphicsCommandList::DrawIndexedInstanced"], "A 1280x720 window titled `20. Model Loading` opens, clears to a neutral dark gray background, and renders a small colored diamond mesh loaded from `assets/sample_model.obj`.", (0.11, 0.11, 0.11), asset_policy="external mesh asset copied to output", status="Implemented", preserve_existing=True),
    _dx12_sample("21", "Scene Graph", "Render multiple objects with separate transforms, materials, and draw calls.", ["scene object list", "per-object constants", "material table", "multiple draw calls"], ["ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable", "ID3D12GraphicsCommandList::IASetVertexBuffers", "ID3D12GraphicsCommandList::IASetIndexBuffer", "ID3D12GraphicsCommandList::DrawIndexedInstanced"], "Scaffold visual: a smoke-test window clears to a dark magenta-gray background. Final target: multi-object scene.", (0.13, 0.08, 0.12)),
    _dx12_sample("22", "Lighting And Materials", "Combine material parameters, normals, and light constants for basic shaded objects.", ["normal vectors", "directional light", "material constants", "Lambert diffuse", "specular controls"], ["D3D12_INPUT_ELEMENT_DESC", "D3DCompileFromFile", "ID3D12Device::CreateGraphicsPipelineState", "ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable"], "Scaffold visual: a smoke-test window clears to a dark brown-gray background. Final target: lit objects with distinct materials.", (0.13, 0.10, 0.08)),
    _dx12_sample("23", "Instancing", "Draw many copies of the same mesh with per-instance data.", ["instance buffer", "per-instance input layout", "instance count", "draw call reduction"], ["ID3D12GraphicsCommandList::IASetVertexBuffers", "ID3D12GraphicsCommandList::DrawInstanced", "ID3D12GraphicsCommandList::DrawIndexedInstanced", "D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA"], "Scaffold visual: a smoke-test window clears to a dark horizon-blue background. Final target: many instanced objects.", (0.04, 0.09, 0.14)),
    _dx12_sample("24", "Shadow Mapping", "Render depth from a light view and sample it to shade shadows.", ["shadow map", "light view projection", "depth-only pass", "shadow comparison"], ["ID3D12Device::CreateDepthStencilView", "ID3D12Device::CreateShaderResourceView", "ID3D12GraphicsCommandList::ClearDepthStencilView", "ID3D12GraphicsCommandList::DrawIndexedInstanced"], "Scaffold visual: a smoke-test window clears to a dark graphite background. Final target: object casting a shadow.", (0.08, 0.09, 0.10)),
    _dx12_sample("25", "Post Processing", "Apply a full-screen post-processing pass to an offscreen render target.", ["full-screen triangle", "post-process pixel shader", "SRV input texture", "two-pass rendering"], ["ID3D12Device::CreateShaderResourceView", "ID3D12GraphicsCommandList::SetDescriptorHeaps", "ID3D12GraphicsCommandList::DrawInstanced", "ID3D12GraphicsCommandList::ResourceBarrier"], "Scaffold visual: a smoke-test window clears to a dark neutral background. Final target: grayscale or color-shift post effect.", (0.09, 0.09, 0.10)),
    _dx12_sample("26", "ImGui Integration", "Integrate Dear ImGui as a debug UI overlay for DX12 samples.", ["ImGui context", "descriptor heap for UI", "frame UI build", "overlay rendering"], ["ImGui::CreateContext", "ImGui_ImplWin32_Init", "ImGui_ImplDX12_Init", "ImGui_ImplDX12_RenderDrawData"], "Scaffold visual: a smoke-test window clears to a dark blue-gray background. Final target: DX12 scene with ImGui overlay.", (0.07, 0.10, 0.13), shader_policy="Dear ImGui shaders handled by backend in final version", asset_policy="external Dear ImGui source planned"),
    _dx12_sample("27", "Mini Renderer", "Combine the learning samples into a small DX12 renderer with frame resources, descriptors, passes, and debug UI.", ["renderer structure", "frame resources", "descriptor management", "render passes", "debug UI"], ["ID3D12GraphicsCommandList::DrawIndexedInstanced", "ID3D12GraphicsCommandList::SetDescriptorHeaps", "ID3D12GraphicsCommandList::ResourceBarrier", "ID3D12CommandQueue::ExecuteCommandLists", "IDXGISwapChain::Present"], "Scaffold visual: a smoke-test window clears to a final-project dark background. Final target: interactive rendered scene.", (0.06, 0.08, 0.11), asset_policy="procedural and external assets planned"),
]


MAIN_TEMPLATE = r'''#include <windows.h>
#include <wrl/client.h>

#include <chrono>
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <string>

#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>

#include "LearningStage.h"

using Microsoft::WRL::ComPtr;

namespace
{{
constexpr uint32_t FrameCount = 2;
constexpr uint32_t WindowWidth = 1280;
constexpr uint32_t WindowHeight = 720;

HWND g_hwnd = nullptr;

struct Dx12Context
{{
    ComPtr<IDXGIFactory4> Factory;
    ComPtr<ID3D12Device> Device;
    ComPtr<ID3D12CommandQueue> CommandQueue;
    ComPtr<IDXGISwapChain3> SwapChain;
    ComPtr<ID3D12DescriptorHeap> RtvHeap;
    ComPtr<ID3D12Resource> RenderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> CommandAllocators[FrameCount];
    ComPtr<ID3D12GraphicsCommandList> CommandList;
    ComPtr<ID3D12Fence> Fence;
    uint64_t FenceValues[FrameCount] = {{}};
    HANDLE FenceEvent = nullptr;
    uint32_t RtvDescriptorSize = 0;
    uint32_t FrameIndex = 0;
}};

void ThrowIfFailed(HRESULT hr, const char* message)
{{
    if (FAILED(hr))
    {{
        throw std::runtime_error(message);
    }}
}}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{{
    switch (message)
    {{
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {{
            PostQuitMessage(0);
            return 0;
        }}
        break;
    default:
        break;
    }}

    return DefWindowProc(hwnd, message, wParam, lParam);
}}

void InitWindow(HINSTANCE instance, int showCommand)
{{
    const wchar_t* className = L"{class_name}WindowClass";

    WNDCLASSEXW windowClass = {{}};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.lpszClassName = className;
    ThrowIfFailed(RegisterClassExW(&windowClass) ? S_OK : E_FAIL, "RegisterClassExW failed.");

    RECT rect = {{ 0, 0, static_cast<LONG>(WindowWidth), static_cast<LONG>(WindowHeight) }};
    ThrowIfFailed(AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE) ? S_OK : E_FAIL, "AdjustWindowRect failed.");

    g_hwnd = CreateWindowExW(
        0,
        className,
        L"{display_name}",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        instance,
        nullptr);

    ThrowIfFailed(g_hwnd ? S_OK : E_FAIL, "CreateWindowExW failed.");
    ShowWindow(g_hwnd, showCommand);
}}

ComPtr<IDXGIAdapter1> ChooseHardwareAdapter(IDXGIFactory4* factory)
{{
    ComPtr<IDXGIAdapter1> adapter;

    for (UINT index = 0; factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND; ++index)
    {{
        DXGI_ADAPTER_DESC1 desc = {{}};
        adapter->GetDesc1(&desc);

        if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
        {{
            continue;
        }}

        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
        {{
            return adapter;
        }}
    }}

    return nullptr;
}}

void InitGraphicsDevice(Dx12Context& dx)
{{
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {{
        debugController->EnableDebugLayer();
    }}
#endif

    UINT factoryFlags = 0;
#if defined(_DEBUG)
    factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
    ThrowIfFailed(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&dx.Factory)), "CreateDXGIFactory2 failed.");

    ComPtr<IDXGIAdapter1> adapter = ChooseHardwareAdapter(dx.Factory.Get());
    ThrowIfFailed(adapter ? S_OK : E_FAIL, "No suitable DX12 hardware adapter found.");

    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dx.Device)), "D3D12CreateDevice failed.");

    D3D12_COMMAND_QUEUE_DESC queueDesc = {{}};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(dx.Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&dx.CommandQueue)), "CreateCommandQueue failed.");

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {{}};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = WindowWidth;
    swapChainDesc.Height = WindowHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(
        dx.Factory->CreateSwapChainForHwnd(dx.CommandQueue.Get(), g_hwnd, &swapChainDesc, nullptr, nullptr, &swapChain),
        "CreateSwapChainForHwnd failed.");
    ThrowIfFailed(dx.Factory->MakeWindowAssociation(g_hwnd, DXGI_MWA_NO_ALT_ENTER), "MakeWindowAssociation failed.");
    ThrowIfFailed(swapChain.As(&dx.SwapChain), "IDXGISwapChain3 query failed.");
    dx.FrameIndex = dx.SwapChain->GetCurrentBackBufferIndex();
}}

void CreateCommonResources(Dx12Context& dx)
{{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {{}};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(dx.Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&dx.RtvHeap)), "CreateDescriptorHeap failed.");

    dx.RtvDescriptorSize = dx.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dx.RtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (uint32_t i = 0; i < FrameCount; ++i)
    {{
        ThrowIfFailed(dx.SwapChain->GetBuffer(i, IID_PPV_ARGS(&dx.RenderTargets[i])), "GetBuffer failed.");
        dx.Device->CreateRenderTargetView(dx.RenderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += dx.RtvDescriptorSize;

        ThrowIfFailed(
            dx.Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&dx.CommandAllocators[i])),
            "CreateCommandAllocator failed.");
    }}

    ThrowIfFailed(
        dx.Device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            dx.CommandAllocators[dx.FrameIndex].Get(),
            nullptr,
            IID_PPV_ARGS(&dx.CommandList)),
        "CreateCommandList failed.");
    ThrowIfFailed(dx.CommandList->Close(), "Initial command list close failed.");

    ThrowIfFailed(dx.Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dx.Fence)), "CreateFence failed.");
    dx.FenceValues[dx.FrameIndex] = 1;
    dx.FenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    ThrowIfFailed(dx.FenceEvent ? S_OK : E_FAIL, "CreateEventW failed.");
}}

D3D12_CPU_DESCRIPTOR_HANDLE CurrentRtvHandle(const Dx12Context& dx)
{{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = dx.RtvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(dx.FrameIndex) * dx.RtvDescriptorSize;
    return handle;
}}

void BeginFrame(Dx12Context& dx)
{{
    ThrowIfFailed(dx.CommandAllocators[dx.FrameIndex]->Reset(), "CommandAllocator reset failed.");
    ThrowIfFailed(dx.CommandList->Reset(dx.CommandAllocators[dx.FrameIndex].Get(), nullptr), "CommandList reset failed.");

    D3D12_RESOURCE_BARRIER barrier = {{}};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = dx.RenderTargets[dx.FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    dx.CommandList->ResourceBarrier(1, &barrier);

    const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CurrentRtvHandle(dx);
    dx.CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
}}

void Render(Dx12Context& dx, LearningStageState& stage)
{{
    BeginFrame(dx);

    LearningStageRenderContext stageContext = {{}};
    stageContext.CommandList = dx.CommandList.Get();
    stageContext.RenderTargetView = CurrentRtvHandle(dx);
    ApplyStageSpecificRender(stage, stageContext);

    D3D12_RESOURCE_BARRIER barrier = {{}};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = dx.RenderTargets[dx.FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    dx.CommandList->ResourceBarrier(1, &barrier);

    ThrowIfFailed(dx.CommandList->Close(), "CommandList close failed.");

    ID3D12CommandList* commandLists[] = {{ dx.CommandList.Get() }};
    dx.CommandQueue->ExecuteCommandLists(1, commandLists);
}}

void MoveToNextFrame(Dx12Context& dx)
{{
    const uint64_t currentFenceValue = dx.FenceValues[dx.FrameIndex];
    ThrowIfFailed(dx.CommandQueue->Signal(dx.Fence.Get(), currentFenceValue), "Fence signal failed.");

    dx.FrameIndex = dx.SwapChain->GetCurrentBackBufferIndex();

    if (dx.Fence->GetCompletedValue() < dx.FenceValues[dx.FrameIndex])
    {{
        ThrowIfFailed(dx.Fence->SetEventOnCompletion(dx.FenceValues[dx.FrameIndex], dx.FenceEvent), "SetEventOnCompletion failed.");
        WaitForSingleObject(dx.FenceEvent, INFINITE);
    }}

    dx.FenceValues[dx.FrameIndex] = currentFenceValue + 1;
}}

void WaitForGpu(Dx12Context& dx)
{{
    ThrowIfFailed(dx.CommandQueue->Signal(dx.Fence.Get(), dx.FenceValues[dx.FrameIndex]), "Fence signal failed.");
    ThrowIfFailed(dx.Fence->SetEventOnCompletion(dx.FenceValues[dx.FrameIndex], dx.FenceEvent), "SetEventOnCompletion failed.");
    WaitForSingleObject(dx.FenceEvent, INFINITE);
    ++dx.FenceValues[dx.FrameIndex];
}}

void EndFrame(Dx12Context& dx)
{{
    ThrowIfFailed(dx.SwapChain->Present(1, 0), "Present failed.");
    MoveToNextFrame(dx);
}}

void CleanupCommonResources(Dx12Context& dx)
{{
    WaitForGpu(dx);

    if (dx.FenceEvent)
    {{
        CloseHandle(dx.FenceEvent);
        dx.FenceEvent = nullptr;
    }}
}}

int Run(HINSTANCE instance, int showCommand)
{{
    InitWindow(instance, showCommand);

    Dx12Context dx = {{}};
    LearningStageState stage = {{}};

    InitGraphicsDevice(dx);
    CreateCommonResources(dx);
    ApplyStageSpecificSetup(stage, dx.Device.Get());

    auto startTime = std::chrono::steady_clock::now();

    MSG msg = {{}};
    while (msg.message != WM_QUIT)
    {{
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {{
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }}

        const auto now = std::chrono::steady_clock::now();
        const double timeSeconds = std::chrono::duration<double>(now - startTime).count();

        UpdateStageSpecificDemo(stage, timeSeconds);
        Render(dx, stage);
        EndFrame(dx);
    }}

    ApplyStageSpecificCleanup(stage);
    CleanupCommonResources(dx);
    return static_cast<int>(msg.wParam);
}}
}}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{{
    try
    {{
        return Run(instance, showCommand);
    }}
    catch (const std::exception& ex)
    {{
        std::wstring message(ex.what(), ex.what() + std::strlen(ex.what()));
        MessageBoxW(nullptr, message.c_str(), L"{display_name} Error", MB_OK | MB_ICONERROR);
        return -1;
    }}
}}
'''


def project_guid(sample: dict[str, object]) -> str:
    return "{" + str(uuid.uuid5(GUID_NAMESPACE, f"{sample['number']}.{sample['name']}")).upper() + "}"


def folder_name(sample: dict[str, object]) -> str:
    return f"{sample['number']}. {sample['name']}"


def root_namespace(sample: dict[str, object]) -> str:
    return "".join(ch for ch in str(sample["name"]) if ch.isalnum())


def markdown_list(items: list[str]) -> str:
    return "\n".join(f"- {item}" for item in items)


def project_file(sample: dict[str, object], guid: str) -> str:
    display = folder_name(sample)
    shader_name = f"{sample['name']}.hlsl"
    ns = root_namespace(sample)
    return f'''<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <Keyword>Win32Proj</Keyword>
    <ProjectGuid>{guid}</ProjectGuid>
    <RootNamespace>{ns}</RootNamespace>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v145</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v145</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings" />
  <ImportGroup Label="Shared" />
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup>
    <OutDir>$(ProjectDir)bin\\$(Platform)\\$(Configuration)\\</OutDir>
    <IntDir>$(ProjectDir)obj\\$(Platform)\\$(Configuration)\\</IntDir>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <WarningLevel>Level4</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;_DEBUG;UNICODE;_UNICODE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp17</LanguageStandard>
    </ClCompile>
    <Link>
      <SubSystem>Windows</SubSystem>
      <AdditionalDependencies>d3d12.lib;dxgi.lib;dxguid.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level4</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;NDEBUG;UNICODE;_UNICODE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp17</LanguageStandard>
    </ClCompile>
    <Link>
      <SubSystem>Windows</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <AdditionalDependencies>d3d12.lib;dxgi.lib;dxguid.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="main.cpp" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="LearningStage.h" />
  </ItemGroup>
  <ItemGroup>
    <None Include="README.md" />
    <None Include="assets\\README.md" />
    <None Include="shaders\\README.md" />
    <None Include="shaders\\{shader_name}" />
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets" />
</Project>
'''


def filters_file(sample: dict[str, object]) -> str:
    shader_name = f"{sample['name']}.hlsl"
    return f'''<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="Source Files">
      <UniqueIdentifier>{{0f08dc8d-efb2-4c3e-bca5-773630652efe}}</UniqueIdentifier>
      <Extensions>cpp;c;cc;cxx</Extensions>
    </Filter>
    <Filter Include="Header Files">
      <UniqueIdentifier>{{602ff439-cbbf-4fb0-8eaf-d3093f9235e3}}</UniqueIdentifier>
      <Extensions>h;hpp;hxx</Extensions>
    </Filter>
    <Filter Include="Shaders">
      <UniqueIdentifier>{{d74b7d8f-d40a-40b2-8318-24a149954ca0}}</UniqueIdentifier>
    </Filter>
    <Filter Include="Assets">
      <UniqueIdentifier>{{9407f8a2-4102-4e52-b594-8a6b8473ee99}}</UniqueIdentifier>
    </Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="main.cpp">
      <Filter>Source Files</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="LearningStage.h">
      <Filter>Header Files</Filter>
    </ClInclude>
  </ItemGroup>
  <ItemGroup>
    <None Include="README.md" />
    <None Include="shaders\\README.md">
      <Filter>Shaders</Filter>
    </None>
    <None Include="shaders\\{shader_name}">
      <Filter>Shaders</Filter>
    </None>
    <None Include="assets\\README.md">
      <Filter>Assets</Filter>
    </None>
  </ItemGroup>
</Project>
'''


def stage_header(sample: dict[str, object]) -> str:
    r, g, b = sample["color"]
    status_note = "This scaffold intentionally keeps the current render hook to a clear command until this lesson is implemented."
    if sample["status"] == "Implemented":
        status_note = "This first sample intentionally creates no stage-owned GPU resources."
    return f'''#pragma once

// Learning goal: {sample["goal"]}
// Implementation status: {sample["status"]}.

#include <d3d12.h>

struct LearningStageState
{{
    float ClearColor[4] = {{ {r:.2f}f, {g:.2f}f, {b:.2f}f, 1.0f }};
}};

struct LearningStageRenderContext
{{
    ID3D12GraphicsCommandList* CommandList = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = {{}};
}};

inline void ApplyStageSpecificSetup(LearningStageState& stage, ID3D12Device* device)
{{
    (void)stage;
    (void)device;
    // {status_note}
}}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{{
    (void)timeSeconds;
    stage.ClearColor[0] = {r:.2f}f;
    stage.ClearColor[1] = {g:.2f}f;
    stage.ClearColor[2] = {b:.2f}f;
    stage.ClearColor[3] = 1.0f;
}}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{{
    context.CommandList->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
}}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{{
    (void)stage;
}}
'''


def readme_file(sample: dict[str, object]) -> str:
    display = folder_name(sample)
    shader_name = f"{sample['name']}.hlsl"
    controls = sample.get("controls", "Press `Esc` or close the window to exit.")
    prerequisite = "none" if sample["number"] == "01" else f"{int(sample['number']) - 1:02d}. {SAMPLES[int(sample['number']) - 2]['name']}"
    return f'''# {display}

## Intent
{sample["goal"]}

## Implementation Status
{sample["status"]}. {"This project currently provides the required folder/project structure and a DX12 clear/present smoke test; the lesson-specific rendering work is intentionally left for the focused implementation pass." if sample["status"] != "Implemented" else "This project implements its stated first-step goal."}

## Prerequisite Sample
{prerequisite}

## New Concepts
{markdown_list(sample["new_concepts"])}

## Expected Result
{sample["expected"]}

## Important API Objects / Calls
{markdown_list(sample["api_calls"])}

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `{shader_name}` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
{controls}

## Verification
Build command:

```powershell
& "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe" ".\\{display}\\{display}.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe" ".\\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, clears the back buffer to the documented scaffold color, and continues presenting until closed.

## Intent Match Checklist
- [ ] The project builds independently.
- [ ] The visual result demonstrates the stated intent.
- [ ] Every new API call listed above appears in code.
- [ ] No unrelated concept is introduced as a required dependency.
'''


def shaders_readme(sample: dict[str, object]) -> str:
    return f'''# Shaders

Policy: {sample["shader_policy"]}.

The current scaffold keeps this folder present for repository consistency. When the lesson-specific implementation is completed, shader source for this sample should live here unless the sample README states a different policy.
'''


def shader_file(sample: dict[str, object]) -> str:
    return f'''// Placeholder shader file for {folder_name(sample)}.
// Implementation status: {sample["status"]}.
// Lesson goal: {sample["goal"]}
'''


def assets_readme(sample: dict[str, object]) -> str:
    return f'''# Assets

Policy: {sample["asset_policy"]}.

This scaffold has no required runtime asset copy step unless the sample-specific implementation later introduces one.
'''


def solution_file() -> str:
    lines = [
        "Microsoft Visual Studio Solution File, Format Version 12.00",
        "# Visual Studio Version 18",
        "VisualStudioVersion = 18.0.36231.0",
        "MinimumVisualStudioVersion = 10.0.40219.1",
    ]
    for sample in SAMPLES:
        display = folder_name(sample)
        guid = project_guid(sample)
        lines.append(f'Project("{SOLUTION_GUID_CPP}") = "{display}", "{display}\\{display}.vcxproj", "{guid}"')
        lines.append("EndProject")
    lines.extend([
        "Global",
        "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution",
        "\t\tDebug|x64 = Debug|x64",
        "\t\tRelease|x64 = Release|x64",
        "\tEndGlobalSection",
        "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution",
    ])
    for sample in SAMPLES:
        guid = project_guid(sample)
        lines.extend([
            f"\t\t{guid}.Debug|x64.ActiveCfg = Debug|x64",
            f"\t\t{guid}.Debug|x64.Build.0 = Debug|x64",
            f"\t\t{guid}.Release|x64.ActiveCfg = Release|x64",
            f"\t\t{guid}.Release|x64.Build.0 = Release|x64",
        ])
    lines.extend([
        "\tEndGlobalSection",
        "\tGlobalSection(SolutionProperties) = preSolution",
        "\t\tHideSolutionNode = FALSE",
        "\tEndGlobalSection",
        "EndGlobal",
        "",
    ])
    return "\n".join(lines)


def roadmap_file() -> str:
    rows = []
    for sample in SAMPLES:
        rows.append(f"| {sample['number']} | `{sample['name']}` | {sample['goal']} | {sample['status']} |")
    next_sample = next((sample for sample in SAMPLES if sample["status"] != "Implemented"), None)
    if next_sample:
        next_step = f'''Start with `{folder_name(next_sample)}` and implement its focused DX12 lesson:

- promote the sample README from `Scaffold` to `Implemented`
- replace the clear-only render hook with the sample-specific resources and draw/dispatch work
- keep the project independently buildable
- update `ROADMAP.md` and `WORKSPACE_CONTEXT.md`
'''
    else:
        next_step = "All listed samples are implemented. Use this roadmap for review, refactoring, and final project polish."
    return f'''# DirectX 12 Learning Roadmap

This roadmap is organized around DirectX 12's explicit device, resource, descriptor, command-list, synchronization, and render-pass model. It keeps the 27-step learning scale of the DirectX 11 practice set, but the order and topic boundaries are DX12-specific.

## Learning Principles

- One sample, one primary concept.
- Keep the first important DX12 objects explicit in `main.cpp`.
- Put only sample-specific resources and hooks in `LearningStage.h`.
- Prefer deterministic procedural data before asset-loading samples.
- Every sample must have a visible result that can be checked quickly.
- Every sample must build independently and also belong to `DirectX12Learning.sln`.

## Full Sequence

| No. | Sample | Goal | Status |
|---|---|---|---|
{chr(10).join(rows)}

## Implementation Strategy

`01. Dx12Basic` is implemented as the foundation. The remaining projects are scaffolded as independent DX12 projects with the required folder contract, README contract, shader/assets folders, and a clear/present smoke test. Each scaffold should be promoted from `Scaffold` to `Implemented` one at a time, following its README intent and checklist.

## Next Implementation Pass

{next_step}
'''


def workspace_context_file() -> str:
    sample_lines = "\n".join(f"- `{folder_name(sample)}`: {sample['status']}" for sample in SAMPLES)
    implemented_count = sum(1 for sample in SAMPLES if sample["status"] == "Implemented")
    scaffold_count = len(SAMPLES) - implemented_count
    return f'''# DirectX 12 Practice Workspace Context

## Purpose

This workspace is a DirectX 12 learning repository built around small, repeatable Visual C++ sample projects. Each sample isolates one graphics concept and keeps a consistent file/documentation contract so later samples are easy to compare.

## Current State

- Root solution: `DirectX12Learning.sln`
- Current API focus: DirectX 12 on Win32
- Current platform: Windows, x64
- Current Visual Studio toolset: `v145`
- Project count: {len(SAMPLES)}
- Implemented sample count: {implemented_count}
- Scaffolded sample count: {scaffold_count}

## Samples

{sample_lines}

## Repository Layout Contract

```text
NN. SampleName/
  NN. SampleName.vcxproj
  NN. SampleName.vcxproj.filters
  main.cpp
  LearningStage.h
  shaders/
    README.md
    SampleName.hlsl
  assets/
    README.md
  README.md
```

## Sample Contract

`main.cpp` owns the application shell:

- Win32 window creation
- DX12 device and swap chain setup
- frame timing and message loop
- command submission, present, and synchronization
- calls to the stage hooks

`LearningStage.h` owns only sample-specific code:

- stage resources
- setup/update/render/cleanup hooks
- comments around the one new concept introduced by the sample

## Build Notes

Build the whole workspace:

```powershell
& "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe" ".\\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Generated build outputs are placed under each sample's `bin/` and `obj/` directories and are ignored by git.

## Expansion Rules

- Add each new sample project to `DirectX12Learning.sln`.
- Keep each sample independently buildable from its `.vcxproj`.
- Keep one primary learning goal per sample.
- Prefer procedural data for early samples.
- Use external HLSL files once pipeline state and shaders become the primary lesson.
- Document placeholders explicitly in both code and README files.
'''


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(text.replace("\n", "\r\n").encode("utf-8"))


def main() -> None:
    for sample in SAMPLES:
        display = folder_name(sample)
        sample_dir = ROOT / display
        guid = project_guid(sample)
        class_name = f"Dx12{sample['number']}{root_namespace(sample)}"
        shader_name = f"{sample['name']}.hlsl"

        if not sample.get("preserve_existing") or not sample_dir.exists():
            write_text(sample_dir / f"{display}.vcxproj", project_file(sample, guid))
            write_text(sample_dir / f"{display}.vcxproj.filters", filters_file(sample))
            write_text(sample_dir / "main.cpp", MAIN_TEMPLATE.format(class_name=class_name, display_name=display))
            write_text(sample_dir / "LearningStage.h", stage_header(sample))
            write_text(sample_dir / "README.md", readme_file(sample))
            write_text(sample_dir / "shaders" / "README.md", shaders_readme(sample))
            write_text(sample_dir / "shaders" / shader_name, shader_file(sample))
            write_text(sample_dir / "assets" / "README.md", assets_readme(sample))

    write_text(ROOT / "DirectX12Learning.sln", solution_file())
    write_text(ROOT / "ROADMAP.md", roadmap_file())
    write_text(ROOT / "WORKSPACE_CONTEXT.md", workspace_context_file())


if __name__ == "__main__":
    main()
