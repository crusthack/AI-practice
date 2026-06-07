#pragma once

// Learning goal: Compare small push constants with uniform buffer binding.
// Implementation status: Implemented.

#include <windows.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
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

struct UniformData
{
    float Offset[2];
    float Padding[2];
};

struct PushConstants
{
    float Tint[4];
};

inline const std::vector<Vertex>& QuadVertices()
{
    static const std::vector<Vertex> vertices = {
        {{ -0.55f, -0.45f }, { 1.0f, 0.25f, 0.20f }},
        {{ 0.55f, -0.45f }, { 0.25f, 0.85f, 0.35f }},
        {{ 0.55f, 0.45f }, { 0.25f, 0.45f, 1.0f }},
        {{ -0.55f, 0.45f }, { 1.0f, 0.85f, 0.25f }},
    };
    return vertices;
}

inline const std::vector<uint16_t>& QuadIndices()
{
    static const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };
    return indices;
}

struct LearningStageState
{
    float ClearColor[4] = { 0.06f, 0.18f, 0.11f, 1.0f };
    VkDevice Device = VK_NULL_HANDLE;
    VkBuffer VertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory VertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer IndexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory IndexBufferMemory = VK_NULL_HANDLE;
    VkBuffer UniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory UniformBufferMemory = VK_NULL_HANDLE;
    void* UniformMapped = nullptr;
    VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline GraphicsPipeline = VK_NULL_HANDLE;
    PushConstants Push = {{ 1.0f, 0.92f, 0.82f, 1.0f }};
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
        if ((typeFilter & (1u << i)) != 0 &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
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

inline void UploadToDeviceLocalBuffer(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue queue,
    const void* sourceData,
    VkDeviceSize size,
    VkBufferUsageFlags finalUsage,
    VkBuffer& destination,
    VkDeviceMemory& destinationMemory)
{
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    CreateBuffer(physicalDevice, device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

    void* mapped = nullptr;
    StageThrowIfFailed(vkMapMemory(device, stagingMemory, 0, size, 0, &mapped), "vkMapMemory failed.");
    std::memcpy(mapped, sourceData, static_cast<size_t>(size));
    vkUnmapMemory(device, stagingMemory);

    CreateBuffer(physicalDevice, device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | finalUsage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, destination, destinationMemory);
    CopyBuffer(device, commandPool, queue, stagingBuffer, destination, size);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);
}

inline void CreateGeometryBuffers(LearningStageState& stage, VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue queue)
{
    const auto& vertices = QuadVertices();
    UploadToDeviceLocalBuffer(physicalDevice, device, commandPool, queue, vertices.data(),
        sizeof(vertices[0]) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, stage.VertexBuffer, stage.VertexBufferMemory);

    const auto& indices = QuadIndices();
    UploadToDeviceLocalBuffer(physicalDevice, device, commandPool, queue, indices.data(),
        sizeof(indices[0]) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, stage.IndexBuffer, stage.IndexBufferMemory);
}

inline void CreateDescriptorResources(LearningStageState& stage, VkPhysicalDevice physicalDevice, VkDevice device)
{
    VkDescriptorSetLayoutBinding uniformBinding = {};
    uniformBinding.binding = 0;
    uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBinding.descriptorCount = 1;
    uniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uniformBinding;
    StageThrowIfFailed(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &stage.DescriptorSetLayout), "vkCreateDescriptorSetLayout failed.");

    CreateBuffer(physicalDevice, device, sizeof(UniformData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stage.UniformBuffer, stage.UniformBufferMemory);
    StageThrowIfFailed(vkMapMemory(device, stage.UniformBufferMemory, 0, sizeof(UniformData), 0, &stage.UniformMapped), "vkMapMemory failed.");

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    StageThrowIfFailed(vkCreateDescriptorPool(device, &poolInfo, nullptr, &stage.DescriptorPool), "vkCreateDescriptorPool failed.");

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = stage.DescriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &stage.DescriptorSetLayout;
    StageThrowIfFailed(vkAllocateDescriptorSets(device, &allocateInfo, &stage.DescriptorSet), "vkAllocateDescriptorSets failed.");

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = stage.UniformBuffer;
    bufferInfo.range = sizeof(UniformData);

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = stage.DescriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.pBufferInfo = &bufferInfo;
    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

inline void CreateGraphicsPipeline(LearningStageState& stage, VkDevice device, VkRenderPass renderPass, VkExtent2D extent)
{
    const auto vertexShaderCode = ReadShaderBinary(L"shaders/Push Constants And Uniforms.vert.spv");
    const auto fragmentShaderCode = ReadShaderBinary(L"shaders/Push Constants And Uniforms.frag.spv");
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
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPushConstantRange pushRange = {};
    pushRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushRange.offset = 0;
    pushRange.size = sizeof(PushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &stage.DescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushRange;
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
    CreateGeometryBuffers(stage, physicalDevice, device, commandPool, queue);
    CreateDescriptorResources(stage, physicalDevice, device);
    CreateGraphicsPipeline(stage, device, renderPass, extent);
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.ClearColor[0] = 0.06f;
    stage.ClearColor[1] = 0.18f;
    stage.ClearColor[2] = 0.11f;
    stage.ClearColor[3] = 1.0f;

    UniformData uniform = {};
    uniform.Offset[0] = 0.16f * std::sin(static_cast<float>(timeSeconds));
    uniform.Offset[1] = 0.0f;
    std::memcpy(stage.UniformMapped, &uniform, sizeof(uniform));

    stage.Push.Tint[0] = 0.80f + 0.20f * std::cos(static_cast<float>(timeSeconds));
    stage.Push.Tint[1] = 1.0f;
    stage.Push.Tint[2] = 0.90f;
    stage.Push.Tint[3] = 1.0f;
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
    vkCmdBindDescriptorSets(context.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, stage.PipelineLayout, 0, 1, &stage.DescriptorSet, 0, nullptr);
    vkCmdPushConstants(context.CommandBuffer, stage.PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &stage.Push);
    vkCmdBindVertexBuffers(context.CommandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(context.CommandBuffer, stage.IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(context.CommandBuffer, static_cast<uint32_t>(QuadIndices().size()), 1, 0, 0, 0);
    vkCmdEndRenderPass(context.CommandBuffer);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    if (stage.GraphicsPipeline) vkDestroyPipeline(stage.Device, stage.GraphicsPipeline, nullptr);
    if (stage.PipelineLayout) vkDestroyPipelineLayout(stage.Device, stage.PipelineLayout, nullptr);
    if (stage.DescriptorPool) vkDestroyDescriptorPool(stage.Device, stage.DescriptorPool, nullptr);
    if (stage.DescriptorSetLayout) vkDestroyDescriptorSetLayout(stage.Device, stage.DescriptorSetLayout, nullptr);
    if (stage.UniformMapped) vkUnmapMemory(stage.Device, stage.UniformBufferMemory);
    if (stage.UniformBuffer) vkDestroyBuffer(stage.Device, stage.UniformBuffer, nullptr);
    if (stage.UniformBufferMemory) vkFreeMemory(stage.Device, stage.UniformBufferMemory, nullptr);
    if (stage.IndexBuffer) vkDestroyBuffer(stage.Device, stage.IndexBuffer, nullptr);
    if (stage.IndexBufferMemory) vkFreeMemory(stage.Device, stage.IndexBufferMemory, nullptr);
    if (stage.VertexBuffer) vkDestroyBuffer(stage.Device, stage.VertexBuffer, nullptr);
    if (stage.VertexBufferMemory) vkFreeMemory(stage.Device, stage.VertexBufferMemory, nullptr);
}
