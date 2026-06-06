#pragma once

// Learning goal: Create a Win32 window, initialize the DX12 device/swap chain/command queue, clear a back buffer, and present.
// Implementation status: Implemented.

#include <cmath>
#include <d3d12.h>

struct LearningStageState
{
    float ClearColor[4] = { 0.08f, 0.13f, 0.20f, 1.0f };
};

struct LearningStageRenderContext
{
    ID3D12GraphicsCommandList* CommandList = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = {};
};

inline void ApplyStageSpecificSetup(LearningStageState& stage, ID3D12Device* device)
{
    (void)stage;
    (void)device;
    // This first sample intentionally creates no stage-owned GPU resources.
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    const float t = static_cast<float>(timeSeconds);
    stage.ClearColor[0] = 0.05f + 0.05f * std::sinf(t * 0.7f);
    stage.ClearColor[1] = 0.10f + 0.06f * std::sinf(t * 0.5f + 1.0f);
    stage.ClearColor[2] = 0.20f + 0.10f * std::sinf(t * 0.9f + 2.0f);
    stage.ClearColor[3] = 1.0f;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    context.CommandList->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    (void)stage;
}
