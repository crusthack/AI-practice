#pragma once

// Learning goal: Create shader modules, pipeline layout, graphics pipeline, and draw the first triangle.
// Implementation status: Implemented.

#include <windows.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.h>

struct LearningStageState
{
    float ClearColor[4] = { 0.05f, 0.10f, 0.22f, 1.0f };
    VkDevice Device = VK_NULL_HANDLE;
    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline GraphicsPipeline = VK_NULL_HANDLE;
};

struct LearningStageRenderContext
{
    VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    VkFramebuffer Framebuffer = VK_NULL_HANDLE;
    VkExtent2D Extent = {};
};

inline void StageThrowIfFailed(VkResult result, const char* message)
{
    if (result < 0)
    {
        throw std::runtime_error(message);
    }
}

inline std::filesystem::path ExecutableDirectory()
{
    wchar_t path[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
    {
        throw std::runtime_error("GetModuleFileNameW failed.");
    }

    return std::filesystem::path(path).parent_path();
}

inline std::vector<char> ReadShaderBinary(const wchar_t* relativePath)
{
    const std::filesystem::path path = ExecutableDirectory() / relativePath;
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Failed to open SPIR-V shader file.");
    }

    const std::streamsize size = file.tellg();
    std::vector<char> buffer(static_cast<size_t>(size));
    file.seekg(0);
    file.read(buffer.data(), size);
    return buffer;
}

inline VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule module = VK_NULL_HANDLE;
    StageThrowIfFailed(vkCreateShaderModule(device, &createInfo, nullptr, &module), "vkCreateShaderModule failed.");
    return module;
}

inline void ApplyStageSpecificSetup(
    LearningStageState& stage,
    VkDevice device,
    VkRenderPass renderPass,
    VkExtent2D extent)
{
    stage.Device = device;

    const std::vector<char> vertexShaderCode = ReadShaderBinary(L"shaders/HelloTriangle.vert.spv");
    const std::vector<char> fragmentShaderCode = ReadShaderBinary(L"shaders/HelloTriangle.frag.spv");
    const VkShaderModule vertexShader = CreateShaderModule(device, vertexShaderCode);
    const VkShaderModule fragmentShader = CreateShaderModule(device, fragmentShaderCode);

    VkPipelineShaderStageCreateInfo shaderStages[2] = {};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertexShader;
    shaderStages[0].pName = "main";
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragmentShader;
    shaderStages[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo vertexInput = {};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport = {};
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    StageThrowIfFailed(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &stage.PipelineLayout), "vkCreatePipelineLayout failed.");

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = stage.PipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    StageThrowIfFailed(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &stage.GraphicsPipeline), "vkCreateGraphicsPipelines failed.");

    vkDestroyShaderModule(device, fragmentShader, nullptr);
    vkDestroyShaderModule(device, vertexShader, nullptr);
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)timeSeconds;
    stage.ClearColor[0] = 0.05f;
    stage.ClearColor[1] = 0.10f;
    stage.ClearColor[2] = 0.22f;
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
    vkCmdBindPipeline(context.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, stage.GraphicsPipeline);
    vkCmdDraw(context.CommandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(context.CommandBuffer);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    if (stage.GraphicsPipeline)
    {
        vkDestroyPipeline(stage.Device, stage.GraphicsPipeline, nullptr);
        stage.GraphicsPipeline = VK_NULL_HANDLE;
    }

    if (stage.PipelineLayout)
    {
        vkDestroyPipelineLayout(stage.Device, stage.PipelineLayout, nullptr);
        stage.PipelineLayout = VK_NULL_HANDLE;
    }
}
