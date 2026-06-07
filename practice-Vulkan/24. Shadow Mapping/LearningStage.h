#pragma once

// Learning goal: Render depth from a light view and sample it to shade shadows.
// Implementation status: Scaffold.

#include <vulkan/vulkan.h>

struct LearningStageState
{
    float ClearColor[4] = { 0.08f, 0.09f, 0.10f, 1.0f };
};

struct LearningStageRenderContext
{
    VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    VkFramebuffer Framebuffer = VK_NULL_HANDLE;
    VkExtent2D Extent = {};
};

inline void ApplyStageSpecificSetup(LearningStageState& stage, VkDevice device)
{
    (void)stage;
    (void)device;
    // This scaffold keeps sample-owned resources out of main.cpp until the focused implementation pass.
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)timeSeconds;
    stage.ClearColor[0] = 0.08f;
    stage.ClearColor[1] = 0.09f;
    stage.ClearColor[2] = 0.10f;
    stage.ClearColor[3] = 1.0f;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    VkClearValue clearValue = {};
    clearValue.color.float32[0] = stage.ClearColor[0];
    clearValue.color.float32[1] = stage.ClearColor[1];
    clearValue.color.float32[2] = stage.ClearColor[2];
    clearValue.color.float32[3] = stage.ClearColor[3];

    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = context.RenderPass;
    beginInfo.framebuffer = context.Framebuffer;
    beginInfo.renderArea.extent = context.Extent;
    beginInfo.clearValueCount = 1;
    beginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(context.CommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(context.CommandBuffer);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    (void)stage;
}
