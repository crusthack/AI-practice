#pragma once

// Learning goal: Upload vertex data through a staging buffer and bind a vertex buffer.
// Implementation status: Implemented.

#include <windows.h>

#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.h>

struct Vertex
{
    float Position[2];
    float Color[3];
};

inline const std::vector<Vertex>& TriangleVertices()
{
    static const std::vector<Vertex> vertices = {
        {{ 0.0f, -0.55f }, { 1.0f, 0.25f, 0.20f }},
        {{ 0.55f, 0.45f }, { 0.25f, 0.85f, 0.35f }},
        {{ -0.55f, 0.45f }, { 0.25f, 0.45f, 1.0f }},
    };
    return vertices;
}

struct LearningStageState
{
    float ClearColor[4] = { 0.10f, 0.08f, 0.24f, 1.0f };
    VkDevice Device = VK_NULL_HANDLE;
    VkBuffer VertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory VertexBufferMemory = VK_NULL_HANDLE;
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

inline uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        const bool typeMatches = (typeFilter & (1u << i)) != 0;
        const bool flagsMatch = (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties;
        if (typeMatches && flagsMatch)
        {
            return i;
        }
    }

    throw std::runtime_error("No compatible Vulkan memory type was found.");
}

inline void CreateBuffer(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& memory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    StageThrowIfFailed(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer), "vkCreateBuffer failed.");

    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, properties);
    StageThrowIfFailed(vkAllocateMemory(device, &allocateInfo, nullptr, &memory), "vkAllocateMemory failed.");
    StageThrowIfFailed(vkBindBufferMemory(device, buffer, memory, 0), "vkBindBufferMemory failed.");
}

inline void CopyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer source, VkBuffer destination, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = commandPool;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    StageThrowIfFailed(vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer), "vkAllocateCommandBuffers failed.");

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    StageThrowIfFailed(vkBeginCommandBuffer(commandBuffer, &beginInfo), "vkBeginCommandBuffer failed.");

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, source, destination, 1, &copyRegion);
    StageThrowIfFailed(vkEndCommandBuffer(commandBuffer), "vkEndCommandBuffer failed.");

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    StageThrowIfFailed(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE), "vkQueueSubmit failed.");
    StageThrowIfFailed(vkQueueWaitIdle(queue), "vkQueueWaitIdle failed.");

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

inline void CreateVertexBuffer(
    LearningStageState& stage,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue queue)
{
    const auto& vertices = TriangleVertices();
    const VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    CreateBuffer(
        physicalDevice,
        device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingMemory);

    void* mapped = nullptr;
    StageThrowIfFailed(vkMapMemory(device, stagingMemory, 0, bufferSize, 0, &mapped), "vkMapMemory failed.");
    std::memcpy(mapped, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, stagingMemory);

    CreateBuffer(
        physicalDevice,
        device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        stage.VertexBuffer,
        stage.VertexBufferMemory);

    CopyBuffer(device, commandPool, queue, stagingBuffer, stage.VertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);
}

inline void CreateGraphicsPipeline(LearningStageState& stage, VkDevice device, VkRenderPass renderPass, VkExtent2D extent)
{
    const std::vector<char> vertexShaderCode = ReadShaderBinary(L"shaders/Vertex Buffer Upload.vert.spv");
    const std::vector<char> fragmentShaderCode = ReadShaderBinary(L"shaders/Vertex Buffer Upload.frag.spv");
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

    VkVertexInputBindingDescription binding = {};
    binding.binding = 0;
    binding.stride = sizeof(Vertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributes[2] = {};
    attributes[0].binding = 0;
    attributes[0].location = 0;
    attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributes[0].offset = offsetof(Vertex, Position);
    attributes[1].binding = 0;
    attributes[1].location = 1;
    attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[1].offset = offsetof(Vertex, Color);

    VkPipelineVertexInputStateCreateInfo vertexInput = {};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &binding;
    vertexInput.vertexAttributeDescriptionCount = 2;
    vertexInput.pVertexAttributeDescriptions = attributes;

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

inline void ApplyStageSpecificSetup(
    LearningStageState& stage,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkRenderPass renderPass,
    VkExtent2D extent,
    VkCommandPool commandPool,
    VkQueue queue)
{
    stage.Device = device;
    CreateVertexBuffer(stage, physicalDevice, device, commandPool, queue);
    CreateGraphicsPipeline(stage, device, renderPass, extent);
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)timeSeconds;
    stage.ClearColor[0] = 0.10f;
    stage.ClearColor[1] = 0.08f;
    stage.ClearColor[2] = 0.24f;
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

    const VkBuffer vertexBuffers[] = { stage.VertexBuffer };
    const VkDeviceSize offsets[] = { 0 };

    vkCmdBeginRenderPass(context.CommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(context.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, stage.GraphicsPipeline);
    vkCmdBindVertexBuffers(context.CommandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdDraw(context.CommandBuffer, static_cast<uint32_t>(TriangleVertices().size()), 1, 0, 0);
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

    if (stage.VertexBuffer)
    {
        vkDestroyBuffer(stage.Device, stage.VertexBuffer, nullptr);
        stage.VertexBuffer = VK_NULL_HANDLE;
    }

    if (stage.VertexBufferMemory)
    {
        vkFreeMemory(stage.Device, stage.VertexBufferMemory, nullptr);
        stage.VertexBufferMemory = VK_NULL_HANDLE;
    }
}
