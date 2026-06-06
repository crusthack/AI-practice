#pragma once

// Learning goal: Load mesh data from an external file and upload it into GPU buffers.
// Implementation status: Implemented.

#include <wrl/client.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>

using Microsoft::WRL::ComPtr;

struct LearningStageSetupContext
{
    ID3D12Device* Device = nullptr;
    uint32_t Width = 0;
    uint32_t Height = 0;
};

struct LearningStageRenderContext
{
    ID3D12GraphicsCommandList* CommandList = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = {};
    uint32_t Width = 0;
    uint32_t Height = 0;
};

struct ModelVertex
{
    float Position[3];
    float Color[3];
};

struct LoadedMesh
{
    std::vector<ModelVertex> Vertices;
    std::vector<uint16_t> Indices;
};

struct LearningStageState
{
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;
    ComPtr<ID3D12Resource> VertexBuffer;
    ComPtr<ID3D12Resource> IndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};
    D3D12_VIEWPORT Viewport = {};
    D3D12_RECT Scissor = {};
    uint32_t IndexCount = 0;
    float ClearColor[4] = { 0.11f, 0.11f, 0.11f, 1.0f };
};

inline void StageThrowIfFailed(HRESULT hr, const char* message)
{
    if (FAILED(hr))
    {
        throw std::runtime_error(message);
    }
}

inline D3D12_RESOURCE_DESC BufferDesc(uint64_t byteSize)
{
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = byteSize;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    return desc;
}

inline ComPtr<ID3DBlob> CompileStageShader(const wchar_t* path, const char* entryPoint, const char* target)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> bytecode;
    ComPtr<ID3DBlob> errors;
    const HRESULT hr = D3DCompileFromFile(path, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, target, flags, 0, &bytecode, &errors);
    if (FAILED(hr))
    {
        throw std::runtime_error(errors ? static_cast<const char*>(errors->GetBufferPointer()) : "Shader compile failed.");
    }

    return bytecode;
}

inline uint16_t ParseObjIndex(const std::string& token)
{
    const size_t slash = token.find('/');
    const std::string number = slash == std::string::npos ? token : token.substr(0, slash);
    const int parsed = std::stoi(number);
    if (parsed <= 0 || parsed > 65535)
    {
        throw std::runtime_error("OBJ index is out of range.");
    }

    return static_cast<uint16_t>(parsed - 1);
}

inline LoadedMesh LoadSimpleObjMesh(const char* path)
{
    std::ifstream file(path);
    if (!file)
    {
        throw std::runtime_error("Failed to open assets/sample_model.obj.");
    }

    LoadedMesh mesh;
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream stream(line);
        std::string tag;
        stream >> tag;

        if (tag == "v")
        {
            ModelVertex vertex = {};
            stream >> vertex.Position[0] >> vertex.Position[1] >> vertex.Position[2];
            vertex.Color[0] = 0.35f + vertex.Position[0] * 0.35f;
            vertex.Color[1] = 0.55f + vertex.Position[1] * 0.25f;
            vertex.Color[2] = 0.85f;
            mesh.Vertices.push_back(vertex);
        }
        else if (tag == "f")
        {
            std::string a;
            std::string b;
            std::string c;
            stream >> a >> b >> c;
            mesh.Indices.push_back(ParseObjIndex(a));
            mesh.Indices.push_back(ParseObjIndex(b));
            mesh.Indices.push_back(ParseObjIndex(c));
        }
    }

    if (mesh.Vertices.empty() || mesh.Indices.empty())
    {
        throw std::runtime_error("OBJ file did not contain supported v/f data.");
    }

    return mesh;
}

inline ComPtr<ID3D12Resource> CreateUploadBuffer(ID3D12Device* device, const void* data, uint64_t byteSize)
{
    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC desc = BufferDesc(byteSize);

    ComPtr<ID3D12Resource> buffer;
    StageThrowIfFailed(
        device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer)),
        "CreateCommittedResource upload buffer failed.");

    void* mapped = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    StageThrowIfFailed(buffer->Map(0, &readRange, &mapped), "Map upload buffer failed.");
    std::memcpy(mapped, data, static_cast<size_t>(byteSize));
    buffer->Unmap(0, nullptr);
    return buffer;
}

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    ID3D12Device* device = context.Device;
    LoadedMesh mesh = LoadSimpleObjMesh("assets\\sample_model.obj");

    const uint64_t vertexBytes = sizeof(ModelVertex) * mesh.Vertices.size();
    const uint64_t indexBytes = sizeof(uint16_t) * mesh.Indices.size();
    stage.VertexBuffer = CreateUploadBuffer(device, mesh.Vertices.data(), vertexBytes);
    stage.IndexBuffer = CreateUploadBuffer(device, mesh.Indices.data(), indexBytes);

    stage.VertexBufferView.BufferLocation = stage.VertexBuffer->GetGPUVirtualAddress();
    stage.VertexBufferView.SizeInBytes = static_cast<UINT>(vertexBytes);
    stage.VertexBufferView.StrideInBytes = sizeof(ModelVertex);
    stage.IndexBufferView.BufferLocation = stage.IndexBuffer->GetGPUVirtualAddress();
    stage.IndexBufferView.SizeInBytes = static_cast<UINT>(indexBytes);
    stage.IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    stage.IndexCount = static_cast<uint32_t>(mesh.Indices.size());

    D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
    rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> errors;
    StageThrowIfFailed(
        D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errors),
        errors ? static_cast<const char*>(errors->GetBufferPointer()) : "D3D12SerializeRootSignature failed.");
    StageThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&stage.RootSignature)), "CreateRootSignature failed.");

    ComPtr<ID3DBlob> vertexShader = CompileStageShader(L"shaders\\Model Loading.hlsl", "VSMain", "vs_5_0");
    ComPtr<ID3DBlob> pixelShader = CompileStageShader(L"shaders\\Model Loading.hlsl", "PSMain", "ps_5_0");

    D3D12_INPUT_ELEMENT_DESC inputElements[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = stage.RootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.InputLayout = { inputElements, _countof(inputElements) };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&stage.PipelineState)), "CreateGraphicsPipelineState failed.");

    stage.Viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    stage.Scissor = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)stage;
    (void)timeSeconds;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    ID3D12GraphicsCommandList* commandList = context.CommandList;
    commandList->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, nullptr);
    commandList->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    commandList->RSSetViewports(1, &stage.Viewport);
    commandList->RSSetScissorRects(1, &stage.Scissor);
    commandList->SetGraphicsRootSignature(stage.RootSignature.Get());
    commandList->SetPipelineState(stage.PipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &stage.VertexBufferView);
    commandList->IASetIndexBuffer(&stage.IndexBufferView);
    commandList->DrawIndexedInstanced(stage.IndexCount, 1, 0, 0, 0);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.IndexBuffer.Reset();
    stage.VertexBuffer.Reset();
    stage.PipelineState.Reset();
    stage.RootSignature.Reset();
}
