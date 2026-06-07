from __future__ import annotations

import uuid
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOLUTION_GUID_CPP = "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"
GUID_NAMESPACE = uuid.UUID("2d178784-8a42-40d5-9ee9-09b31cfa0e29")


SAMPLES = [
    ("01", "VulkanBasic", "Create a Win32 window, Vulkan instance, surface, device, swapchain, command buffers, and clear/present loop.", "Implemented"),
    ("02", "HelloTriangle", "Create shader modules, pipeline layout, graphics pipeline, and draw the first triangle.", "Scaffold"),
    ("03", "Vertex Buffer Upload", "Upload vertex data through a staging buffer and bind a vertex buffer.", "Scaffold"),
    ("04", "Index Buffer", "Draw shared-vertex geometry through an index buffer.", "Scaffold"),
    ("05", "Push Constants And Uniforms", "Compare small push constants with uniform buffer binding.", "Scaffold"),
    ("06", "Descriptor Sets", "Create descriptor set layouts, descriptor pools, and bind uniform descriptors.", "Scaffold"),
    ("07", "Texture Upload", "Upload a procedural texture, transition image layouts, and sample it.", "Scaffold"),
    ("08", "Depth Buffer", "Create a depth image and use depth testing for 3D geometry.", "Scaffold"),
    ("09", "Transform Matrices", "Pass world, view, and projection matrices to shaders.", "Scaffold"),
    ("10", "Camera", "Move a view camera with keyboard and mouse input.", "Scaffold"),
    ("11", "Frames In Flight", "Use per-frame command buffers, semaphores, fences, and uniform buffers.", "Scaffold"),
    ("12", "Image Layout Transitions", "Practice explicit image layout transitions and pipeline barriers.", "Scaffold"),
    ("13", "Staging And Device Local", "Separate CPU-visible staging resources from GPU-local buffers and images.", "Scaffold"),
    ("14", "Pipeline Variants", "Create and switch between multiple graphics pipelines.", "Scaffold"),
    ("15", "Rasterizer State", "Change polygon mode, culling, and front-face settings in pipeline state.", "Scaffold"),
    ("16", "Blend State", "Enable alpha blending and render transparent geometry.", "Scaffold"),
    ("17", "Render To Texture", "Render into an offscreen image and sample it on the swapchain pass.", "Scaffold"),
    ("18", "Compute Shader", "Dispatch a compute shader and synchronize its output.", "Scaffold"),
    ("19", "Storage Buffer And Readback", "Write GPU data through a storage buffer and read selected results on the CPU.", "Scaffold"),
    ("20", "Model Loading", "Load mesh data from an external file and upload it into GPU buffers.", "Scaffold"),
    ("21", "Scene Graph", "Render multiple objects with separate transforms, materials, and draw calls.", "Scaffold"),
    ("22", "Lighting And Materials", "Combine normals, material parameters, and light constants for shaded objects.", "Scaffold"),
    ("23", "Instancing", "Draw many copies of the same mesh with per-instance data.", "Scaffold"),
    ("24", "Shadow Mapping", "Render depth from a light view and sample it to shade shadows.", "Scaffold"),
    ("25", "Post Processing", "Apply a full-screen post-processing pass to an offscreen image.", "Scaffold"),
    ("26", "ImGui Integration", "Integrate Dear ImGui as a debug UI overlay for Vulkan samples.", "Scaffold"),
    ("27", "Mini Renderer", "Combine the lessons into a small Vulkan renderer with passes, descriptors, synchronization, and debug UI.", "Scaffold"),
]


CONCEPTS = {
    "01": ["Win32 window and Vulkan surface", "instance and validation layer selection", "physical/logical device selection", "swapchain images and image views", "render pass, framebuffers, command pool, command buffers", "semaphores, fences, acquire, submit, present"],
    "02": ["SPIR-V shader modules", "pipeline layout", "graphics pipeline", "viewport and scissor", "non-indexed draw call"],
    "03": ["buffer creation", "host-visible staging memory", "device-local vertex buffer", "copy command", "vertex input binding"],
    "04": ["index buffer", "index type", "shared vertices", "indexed draw call"],
    "05": ["push constants", "uniform buffer", "dynamic per-frame data", "alignment rules"],
    "06": ["descriptor set layout", "descriptor pool", "descriptor set allocation", "descriptor writes"],
    "07": ["VkImage", "image memory", "layout transition", "sampler", "sampled image descriptor"],
    "08": ["depth format selection", "depth image", "depth attachment", "depth test state"],
    "09": ["world matrix", "view matrix", "projection matrix", "clip-space conventions"],
    "10": ["camera vectors", "view matrix update", "keyboard input", "mouse look"],
    "11": ["multiple frames in flight", "per-frame fences", "per-frame command buffers", "uniform buffer rotation"],
    "12": ["pipeline barrier", "source/destination stages", "access masks", "image layouts"],
    "13": ["staging buffer", "device-local memory", "transfer queue usage", "copy synchronization"],
    "14": ["pipeline cache mindset", "pipeline state variants", "shader variants", "state switching"],
    "15": ["polygon mode", "cull mode", "front face", "pipeline recreation"],
    "16": ["blend enable", "blend factors", "render ordering", "depth write control"],
    "17": ["offscreen image", "render pass separation", "sampled color attachment", "layout handoff"],
    "18": ["compute pipeline", "dispatch", "storage buffer/image", "compute-to-graphics synchronization"],
    "19": ["storage buffer", "host readback memory", "buffer barriers", "CPU verification"],
    "20": ["mesh parsing", "vertex/index upload", "asset paths", "model bounds"],
    "21": ["scene object list", "per-object transforms", "materials", "multiple draw calls"],
    "22": ["normal vectors", "directional light", "material constants", "per-pixel lighting"],
    "23": ["instance buffer", "per-instance attributes", "instance count", "draw call reduction"],
    "24": ["shadow map", "depth-only pass", "light view-projection", "shadow comparison"],
    "25": ["full-screen triangle", "post-process shader", "input attachment or sampled image", "two-pass rendering"],
    "26": ["ImGui context", "descriptor pool for UI", "frame UI build", "overlay draw data"],
    "27": ["renderer modules", "resource lifetime", "render graph mindset", "debug UI", "frame orchestration"],
}


API_CALLS = {
    "01": ["vkCreateInstance", "vkCreateWin32SurfaceKHR", "vkEnumeratePhysicalDevices", "vkCreateDevice", "vkCreateSwapchainKHR", "vkCreateRenderPass", "vkAllocateCommandBuffers", "vkQueueSubmit", "vkQueuePresentKHR"],
    "02": ["vkCreateShaderModule", "vkCreatePipelineLayout", "vkCreateGraphicsPipelines", "vkCmdDraw"],
    "03": ["vkCreateBuffer", "vkAllocateMemory", "vkMapMemory", "vkCmdCopyBuffer", "vkCmdBindVertexBuffers"],
    "04": ["vkCmdBindIndexBuffer", "vkCmdDrawIndexed"],
    "05": ["vkCmdPushConstants", "vkCreateBuffer", "vkUpdateDescriptorSets"],
    "06": ["vkCreateDescriptorSetLayout", "vkCreateDescriptorPool", "vkAllocateDescriptorSets", "vkUpdateDescriptorSets"],
    "07": ["vkCreateImage", "vkCreateImageView", "vkCreateSampler", "vkCmdPipelineBarrier"],
    "08": ["vkCreateImage", "vkCreateRenderPass", "vkCmdClearDepthStencilImage"],
    "09": ["vkMapMemory", "vkFlushMappedMemoryRanges", "vkCmdBindDescriptorSets"],
    "10": ["GetAsyncKeyState", "vkMapMemory", "vkCmdBindDescriptorSets"],
    "11": ["vkCreateFence", "vkCreateSemaphore", "vkWaitForFences", "vkResetFences"],
    "12": ["vkCmdPipelineBarrier", "VkImageMemoryBarrier", "VkBufferMemoryBarrier"],
    "13": ["vkCmdCopyBuffer", "vkCmdCopyBufferToImage", "vkQueueSubmit"],
    "14": ["vkCreateGraphicsPipelines", "vkCmdBindPipeline"],
    "15": ["VkPipelineRasterizationStateCreateInfo", "vkCreateGraphicsPipelines"],
    "16": ["VkPipelineColorBlendAttachmentState", "vkCreateGraphicsPipelines"],
    "17": ["vkCreateFramebuffer", "vkCmdBeginRenderPass", "vkCmdPipelineBarrier"],
    "18": ["vkCreateComputePipelines", "vkCmdDispatch", "vkCmdPipelineBarrier"],
    "19": ["vkCmdBindDescriptorSets", "vkCmdDispatch", "vkMapMemory"],
    "20": ["vkCreateBuffer", "vkCmdCopyBuffer", "vkCmdDrawIndexed"],
    "21": ["vkCmdBindDescriptorSets", "vkCmdDrawIndexed"],
    "22": ["vkUpdateDescriptorSets", "vkCmdDrawIndexed"],
    "23": ["vkCmdBindVertexBuffers", "vkCmdDrawIndexed"],
    "24": ["vkCreateRenderPass", "vkCmdBeginRenderPass", "vkCmdDrawIndexed"],
    "25": ["vkCmdBeginRenderPass", "vkCmdDraw", "vkCmdPipelineBarrier"],
    "26": ["ImGui_ImplVulkan_Init", "ImGui_ImplVulkan_RenderDrawData"],
    "27": ["vkQueueSubmit", "vkCmdBeginRenderPass", "vkCmdBindDescriptorSets"],
}


COLORS = {
    "01": (0.08, 0.13, 0.20), "02": (0.05, 0.10, 0.22), "03": (0.10, 0.08, 0.24),
    "04": (0.04, 0.16, 0.16), "05": (0.06, 0.18, 0.11), "06": (0.12, 0.14, 0.17),
    "07": (0.07, 0.15, 0.18), "08": (0.15, 0.11, 0.18), "09": (0.04, 0.07, 0.16),
    "10": (0.10, 0.10, 0.12), "11": (0.03, 0.05, 0.09), "12": (0.03, 0.12, 0.08),
    "13": (0.11, 0.13, 0.07), "14": (0.14, 0.10, 0.15), "15": (0.09, 0.12, 0.14),
    "16": (0.16, 0.07, 0.10), "17": (0.11, 0.11, 0.11), "18": (0.07, 0.13, 0.14),
    "19": (0.07, 0.12, 0.06), "20": (0.02, 0.13, 0.13), "21": (0.13, 0.08, 0.12),
    "22": (0.13, 0.10, 0.08), "23": (0.04, 0.09, 0.14), "24": (0.08, 0.09, 0.10),
    "25": (0.09, 0.09, 0.10), "26": (0.05, 0.10, 0.12), "27": (0.10, 0.08, 0.11),
}


MAIN_TEMPLATE = r'''#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#include <windows.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "LearningStage.h"

namespace
{
constexpr uint32_t WindowWidth = 1280;
constexpr uint32_t WindowHeight = 720;

HWND g_hwnd = nullptr;

struct QueueFamilyIndices
{
    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;

    bool IsComplete() const
    {
        return GraphicsFamily.has_value() && PresentFamily.has_value();
    }
};

struct VulkanContext
{
    VkInstance Instance = VK_NULL_HANDLE;
    VkSurfaceKHR Surface = VK_NULL_HANDLE;
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    VkDevice Device = VK_NULL_HANDLE;
    VkQueue GraphicsQueue = VK_NULL_HANDLE;
    VkQueue PresentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
    VkFormat SwapchainFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D SwapchainExtent = {};
    std::vector<VkImage> SwapchainImages;
    std::vector<VkImageView> SwapchainImageViews;
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> Framebuffers;
    VkCommandPool CommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> CommandBuffers;
    VkSemaphore ImageAvailable = VK_NULL_HANDLE;
    VkSemaphore RenderFinished = VK_NULL_HANDLE;
    VkFence InFlight = VK_NULL_HANDLE;
};

void ThrowIfFailed(VkResult result, const char* message)
{
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(message);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
            return 0;
        }
        break;
    default:
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

void InitWindow(HINSTANCE instance, int showCommand)
{
    const wchar_t* className = L"{class_name}WindowClass";

    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    if (!RegisterClassExW(&windowClass))
    {
        throw std::runtime_error("RegisterClassExW failed.");
    }

    RECT rect = { 0, 0, static_cast<LONG>(WindowWidth), static_cast<LONG>(WindowHeight) };
    if (!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE))
    {
        throw std::runtime_error("AdjustWindowRect failed.");
    }

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

    if (!g_hwnd)
    {
        throw std::runtime_error("CreateWindowExW failed.");
    }

    ShowWindow(g_hwnd, showCommand);
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

    for (uint32_t i = 0; i < count; ++i)
    {
        if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            indices.GraphicsFamily = i;
        }

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
        {
            indices.PresentFamily = i;
        }

        if (indices.IsComplete())
        {
            break;
        }
    }

    return indices;
}

bool HasSwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t formatCount = 0;
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    return formatCount > 0 && presentModeCount > 0;
}

VkPhysicalDevice PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t count = 0;
    ThrowIfFailed(vkEnumeratePhysicalDevices(instance, &count, nullptr), "vkEnumeratePhysicalDevices failed.");
    if (count == 0)
    {
        throw std::runtime_error("No Vulkan-capable physical device was found.");
    }

    std::vector<VkPhysicalDevice> devices(count);
    ThrowIfFailed(vkEnumeratePhysicalDevices(instance, &count, devices.data()), "vkEnumeratePhysicalDevices failed.");

    for (VkPhysicalDevice device : devices)
    {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

        bool hasSwapchainExtension = false;
        for (const auto& extension : extensions)
        {
            if (std::strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
            {
                hasSwapchainExtension = true;
                break;
            }
        }

        if (FindQueueFamilies(device, surface).IsComplete() && hasSwapchainExtension && HasSwapchainSupport(device, surface))
        {
            return device;
        }
    }

    throw std::runtime_error("No suitable Vulkan physical device was found.");
}

VkSurfaceFormatKHR ChooseSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());

    for (const auto& format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return formats.front();
}

VkPresentModeKHR ChoosePresentMode(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data());

    for (VkPresentModeKHR mode : modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapchainExtent(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }

    VkExtent2D extent = { WindowWidth, WindowHeight };
    extent.width = max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, extent.width));
    extent.height = max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, extent.height));
    return extent;
}

void InitVulkan(VulkanContext& vk)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "{display_name}";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "VulkanLearning";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    const char* instanceExtensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(std::size(instanceExtensions));
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions;
    ThrowIfFailed(vkCreateInstance(&instanceCreateInfo, nullptr, &vk.Instance), "vkCreateInstance failed.");

    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = GetModuleHandleW(nullptr);
    surfaceCreateInfo.hwnd = g_hwnd;
    ThrowIfFailed(vkCreateWin32SurfaceKHR(vk.Instance, &surfaceCreateInfo, nullptr, &vk.Surface), "vkCreateWin32SurfaceKHR failed.");

    vk.PhysicalDevice = PickPhysicalDevice(vk.Instance, vk.Surface);
    QueueFamilyIndices indices = FindQueueFamilies(vk.PhysicalDevice, vk.Surface);

    std::set<uint32_t> uniqueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    for (uint32_t family : uniqueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = family;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(std::size(deviceExtensions));
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
    ThrowIfFailed(vkCreateDevice(vk.PhysicalDevice, &deviceCreateInfo, nullptr, &vk.Device), "vkCreateDevice failed.");

    vkGetDeviceQueue(vk.Device, indices.GraphicsFamily.value(), 0, &vk.GraphicsQueue);
    vkGetDeviceQueue(vk.Device, indices.PresentFamily.value(), 0, &vk.PresentQueue);

    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.PhysicalDevice, vk.Surface, &capabilities);
    VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(vk.PhysicalDevice, vk.Surface);
    VkPresentModeKHR presentMode = ChoosePresentMode(vk.PhysicalDevice, vk.Surface);
    vk.SwapchainExtent = ChooseSwapchainExtent(vk.PhysicalDevice, vk.Surface);
    vk.SwapchainFormat = surfaceFormat.format;

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = vk.Surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = vk.SwapchainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (indices.GraphicsFamily != indices.PresentFamily)
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swapchainCreateInfo.preTransform = capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    ThrowIfFailed(vkCreateSwapchainKHR(vk.Device, &swapchainCreateInfo, nullptr, &vk.Swapchain), "vkCreateSwapchainKHR failed.");

    vkGetSwapchainImagesKHR(vk.Device, vk.Swapchain, &imageCount, nullptr);
    vk.SwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(vk.Device, vk.Swapchain, &imageCount, vk.SwapchainImages.data());

    vk.SwapchainImageViews.resize(vk.SwapchainImages.size());
    for (size_t i = 0; i < vk.SwapchainImages.size(); ++i)
    {
        VkImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.image = vk.SwapchainImages[i];
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = vk.SwapchainFormat;
        viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.layerCount = 1;
        ThrowIfFailed(vkCreateImageView(vk.Device, &viewCreateInfo, nullptr, &vk.SwapchainImageViews[i]), "vkCreateImageView failed.");
    }

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = vk.SwapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    ThrowIfFailed(vkCreateRenderPass(vk.Device, &renderPassCreateInfo, nullptr, &vk.RenderPass), "vkCreateRenderPass failed.");

    vk.Framebuffers.resize(vk.SwapchainImageViews.size());
    for (size_t i = 0; i < vk.Framebuffers.size(); ++i)
    {
        VkImageView attachments[] = { vk.SwapchainImageViews[i] };
        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = vk.RenderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.width = vk.SwapchainExtent.width;
        framebufferCreateInfo.height = vk.SwapchainExtent.height;
        framebufferCreateInfo.layers = 1;
        ThrowIfFailed(vkCreateFramebuffer(vk.Device, &framebufferCreateInfo, nullptr, &vk.Framebuffers[i]), "vkCreateFramebuffer failed.");
    }

    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = indices.GraphicsFamily.value();
    ThrowIfFailed(vkCreateCommandPool(vk.Device, &commandPoolCreateInfo, nullptr, &vk.CommandPool), "vkCreateCommandPool failed.");

    vk.CommandBuffers.resize(vk.Framebuffers.size());
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = vk.CommandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = static_cast<uint32_t>(vk.CommandBuffers.size());
    ThrowIfFailed(vkAllocateCommandBuffers(vk.Device, &allocateInfo, vk.CommandBuffers.data()), "vkAllocateCommandBuffers failed.");

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    ThrowIfFailed(vkCreateSemaphore(vk.Device, &semaphoreCreateInfo, nullptr, &vk.ImageAvailable), "vkCreateSemaphore failed.");
    ThrowIfFailed(vkCreateSemaphore(vk.Device, &semaphoreCreateInfo, nullptr, &vk.RenderFinished), "vkCreateSemaphore failed.");

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    ThrowIfFailed(vkCreateFence(vk.Device, &fenceCreateInfo, nullptr, &vk.InFlight), "vkCreateFence failed.");
}

void RecordCommandBuffer(VulkanContext& vk, LearningStageState& stage, uint32_t imageIndex)
{
    VkCommandBuffer commandBuffer = vk.CommandBuffers[imageIndex];
    ThrowIfFailed(vkResetCommandBuffer(commandBuffer, 0), "vkResetCommandBuffer failed.");

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    ThrowIfFailed(vkBeginCommandBuffer(commandBuffer, &beginInfo), "vkBeginCommandBuffer failed.");

    LearningStageRenderContext renderContext = {};
    renderContext.CommandBuffer = commandBuffer;
    renderContext.RenderPass = vk.RenderPass;
    renderContext.Framebuffer = vk.Framebuffers[imageIndex];
    renderContext.Extent = vk.SwapchainExtent;
    ApplyStageSpecificRender(stage, renderContext);

    ThrowIfFailed(vkEndCommandBuffer(commandBuffer), "vkEndCommandBuffer failed.");
}

void DrawFrame(VulkanContext& vk, LearningStageState& stage)
{
    vkWaitForFences(vk.Device, 1, &vk.InFlight, VK_TRUE, UINT64_MAX);
    vkResetFences(vk.Device, 1, &vk.InFlight);

    uint32_t imageIndex = 0;
    ThrowIfFailed(vkAcquireNextImageKHR(vk.Device, vk.Swapchain, UINT64_MAX, vk.ImageAvailable, VK_NULL_HANDLE, &imageIndex), "vkAcquireNextImageKHR failed.");
    RecordCommandBuffer(vk, stage, imageIndex);

    VkSemaphore waitSemaphores[] = { vk.ImageAvailable };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { vk.RenderFinished };
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk.CommandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    ThrowIfFailed(vkQueueSubmit(vk.GraphicsQueue, 1, &submitInfo, vk.InFlight), "vkQueueSubmit failed.");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vk.Swapchain;
    presentInfo.pImageIndices = &imageIndex;
    ThrowIfFailed(vkQueuePresentKHR(vk.PresentQueue, &presentInfo), "vkQueuePresentKHR failed.");
}

void Cleanup(VulkanContext& vk)
{
    if (vk.Device)
    {
        vkDeviceWaitIdle(vk.Device);
    }

    if (vk.InFlight) vkDestroyFence(vk.Device, vk.InFlight, nullptr);
    if (vk.RenderFinished) vkDestroySemaphore(vk.Device, vk.RenderFinished, nullptr);
    if (vk.ImageAvailable) vkDestroySemaphore(vk.Device, vk.ImageAvailable, nullptr);
    if (vk.CommandPool) vkDestroyCommandPool(vk.Device, vk.CommandPool, nullptr);
    for (VkFramebuffer framebuffer : vk.Framebuffers) vkDestroyFramebuffer(vk.Device, framebuffer, nullptr);
    if (vk.RenderPass) vkDestroyRenderPass(vk.Device, vk.RenderPass, nullptr);
    for (VkImageView imageView : vk.SwapchainImageViews) vkDestroyImageView(vk.Device, imageView, nullptr);
    if (vk.Swapchain) vkDestroySwapchainKHR(vk.Device, vk.Swapchain, nullptr);
    if (vk.Device) vkDestroyDevice(vk.Device, nullptr);
    if (vk.Surface) vkDestroySurfaceKHR(vk.Instance, vk.Surface, nullptr);
    if (vk.Instance) vkDestroyInstance(vk.Instance, nullptr);
}

int Run(HINSTANCE instance, int showCommand)
{
    InitWindow(instance, showCommand);

    VulkanContext vk = {};
    LearningStageState stage = {};
    InitVulkan(vk);
    ApplyStageSpecificSetup(stage, vk.Device);

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        UpdateStageSpecificDemo(stage, 0.0);
        DrawFrame(vk, stage);
    }

    ApplyStageSpecificCleanup(stage);
    Cleanup(vk);
    return static_cast<int>(msg.wParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    try
    {
        return Run(instance, showCommand);
    }
    catch (const std::exception& ex)
    {
        std::wstring message(ex.what(), ex.what() + std::strlen(ex.what()));
        MessageBoxW(nullptr, message.c_str(), L"{display_name} Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}
'''


def folder(sample: tuple[str, str, str, str]) -> str:
    return f"{sample[0]}. {sample[1]}"


def root_namespace(name: str) -> str:
    return "".join(ch for ch in name if ch.isalnum())


def project_guid(sample: tuple[str, str, str, str]) -> str:
    return "{" + str(uuid.uuid5(GUID_NAMESPACE, f"{sample[0]}.{sample[1]}")).upper() + "}"


def markdown_list(items: list[str]) -> str:
    return "\n".join(f"- {item}" for item in items)


def project_file(sample: tuple[str, str, str, str]) -> str:
    display = folder(sample)
    shader_name = f"{sample[1]}.vert.glsl"
    return f'''<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64"><Configuration>Debug</Configuration><Platform>x64</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64"><Configuration>Release</Configuration><Platform>x64</Platform></ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <Keyword>Win32Proj</Keyword>
    <ProjectGuid>{project_guid(sample)}</ProjectGuid>
    <RootNamespace>{root_namespace(sample[1])}</RootNamespace>
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
    <IncludePath>$(VULKAN_SDK)\\Include;$(IncludePath)</IncludePath>
    <LibraryPath>$(VULKAN_SDK)\\Lib;$(LibraryPath)</LibraryPath>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <WarningLevel>Level4</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;_DEBUG;UNICODE;_UNICODE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp17</LanguageStandard>
    </ClCompile>
    <Link><SubSystem>Windows</SubSystem><AdditionalDependencies>vulkan-1.lib;%(AdditionalDependencies)</AdditionalDependencies></Link>
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
    <Link><SubSystem>Windows</SubSystem><EnableCOMDATFolding>true</EnableCOMDATFolding><OptimizeReferences>true</OptimizeReferences><AdditionalDependencies>vulkan-1.lib;%(AdditionalDependencies)</AdditionalDependencies></Link>
  </ItemDefinitionGroup>
  <ItemGroup><ClCompile Include="main.cpp" /></ItemGroup>
  <ItemGroup><ClInclude Include="LearningStage.h" /></ItemGroup>
  <ItemGroup>
    <None Include="README.md" />
    <None Include="assets\\README.md" />
    <None Include="shaders\\README.md" />
    <None Include="shaders\\{shader_name}" />
    <None Include="shaders\\{sample[1]}.frag.glsl" />
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets" />
</Project>
'''


def filters_file(sample: tuple[str, str, str, str]) -> str:
    return f'''<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="Source Files"><UniqueIdentifier>{{3b4e2e0d-ae07-4be1-bd85-e775c6ecf11c}}</UniqueIdentifier><Extensions>cpp;c;cc;cxx</Extensions></Filter>
    <Filter Include="Header Files"><UniqueIdentifier>{{08d5b09c-0a41-47f7-a490-df72b56d7c76}}</UniqueIdentifier><Extensions>h;hpp;hxx</Extensions></Filter>
    <Filter Include="Shaders"><UniqueIdentifier>{{72ad61f1-98ed-42df-b64c-5968fd743d64}}</UniqueIdentifier></Filter>
    <Filter Include="Assets"><UniqueIdentifier>{{9ac0b10d-bd79-4c34-805c-570a801c7a20}}</UniqueIdentifier></Filter>
  </ItemGroup>
  <ItemGroup><ClCompile Include="main.cpp"><Filter>Source Files</Filter></ClCompile></ItemGroup>
  <ItemGroup><ClInclude Include="LearningStage.h"><Filter>Header Files</Filter></ClInclude></ItemGroup>
  <ItemGroup>
    <None Include="README.md" />
    <None Include="shaders\\README.md"><Filter>Shaders</Filter></None>
    <None Include="shaders\\{sample[1]}.vert.glsl"><Filter>Shaders</Filter></None>
    <None Include="shaders\\{sample[1]}.frag.glsl"><Filter>Shaders</Filter></None>
    <None Include="assets\\README.md"><Filter>Assets</Filter></None>
  </ItemGroup>
</Project>
'''


def stage_header(sample: tuple[str, str, str, str]) -> str:
    r, g, b = COLORS[sample[0]]
    return f'''#pragma once

// Learning goal: {sample[2]}
// Implementation status: {sample[3]}.

#include <vulkan/vulkan.h>

struct LearningStageState
{{
    float ClearColor[4] = {{ {r:.2f}f, {g:.2f}f, {b:.2f}f, 1.0f }};
}};

struct LearningStageRenderContext
{{
    VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    VkFramebuffer Framebuffer = VK_NULL_HANDLE;
    VkExtent2D Extent = {{}};
}};

inline void ApplyStageSpecificSetup(LearningStageState& stage, VkDevice device)
{{
    (void)stage;
    (void)device;
    // { "The first sample intentionally owns no extra stage resources; it only clears the swapchain image." if sample[0] == "01" else "This scaffold keeps sample-owned resources out of main.cpp until the focused implementation pass." }
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
    VkClearValue clearValue = {{}};
    clearValue.color.float32[0] = stage.ClearColor[0];
    clearValue.color.float32[1] = stage.ClearColor[1];
    clearValue.color.float32[2] = stage.ClearColor[2];
    clearValue.color.float32[3] = stage.ClearColor[3];

    VkRenderPassBeginInfo beginInfo = {{}};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = context.RenderPass;
    beginInfo.framebuffer = context.Framebuffer;
    beginInfo.renderArea.extent = context.Extent;
    beginInfo.clearValueCount = 1;
    beginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(context.CommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(context.CommandBuffer);
}}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{{
    (void)stage;
}}
'''


def readme_file(sample: tuple[str, str, str, str]) -> str:
    previous = "none" if sample[0] == "01" else folder(SAMPLES[int(sample[0]) - 2])
    return f'''# {folder(sample)}

## Intent
{sample[2]}

## Implementation Status
{sample[3]}. {"This project implements the first Vulkan clear/present smoke test." if sample[3] == "Implemented" else "This project currently provides the required folder/project structure and a Vulkan clear/present smoke test; lesson-specific rendering is left for the focused implementation pass."}

## Prerequisite Sample
{previous}

## New Concepts
{markdown_list(CONCEPTS[sample[0]])}

## Expected Result
A 1280x720 Win32 window titled `{folder(sample)}` opens, clears the swapchain image to this sample's documented color, and presents until closed.

## Important API Objects / Calls
{markdown_list(API_CALLS[sample[0]])}

## File Map
- `main.cpp`: owns the Win32 window, Vulkan instance/device/swapchain/render-pass/frame loop, synchronization, present, and stage hook calls.
- `LearningStage.h`: owns sample-specific state and render hook code.
- `shaders/`: contains GLSL placeholders for the lesson-specific shader pass.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe" ".\\{folder(sample)}\\{folder(sample)}.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe" ".\\VulkanLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Requires `VULKAN_SDK` to point at an installed Vulkan SDK.

## Intent Match Checklist
- [ ] The project builds independently.
- [ ] The visual result demonstrates the stated intent.
- [ ] Every new API call listed above appears in code.
- [ ] No unrelated concept is introduced as a required dependency.
'''


def shaders_readme(sample: tuple[str, str, str, str]) -> str:
    return f'''# Shaders

This folder reserves shader source for `{folder(sample)}`.

Use GLSL files here and compile them to SPIR-V with `glslc` from the Vulkan SDK when the lesson-specific implementation needs shaders.
'''


def shader_file(sample: tuple[str, str, str, str], kind: str) -> str:
    stage = "vertex" if kind == "vert" else "fragment"
    return f'''// Placeholder {stage} shader for {folder(sample)}.
// Compile with Vulkan SDK glslc when this sample is implemented.
'''


def assets_readme(sample: tuple[str, str, str, str]) -> str:
    if int(sample[0]) < 20:
        policy = "No external runtime assets are required; prefer procedural data while learning the API mechanics."
    else:
        policy = "External assets may be introduced when the sample-specific implementation requires them."
    return f'''# Assets

Policy: {policy}

Document any required copy step or source path here when assets are added.
'''


def solution_file() -> str:
    lines = [
        "Microsoft Visual Studio Solution File, Format Version 12.00",
        "# Visual Studio Version 18",
        "VisualStudioVersion = 18.0.36231.0",
        "MinimumVisualStudioVersion = 10.0.40219.1",
    ]
    for sample in SAMPLES:
        display = folder(sample)
        guid = project_guid(sample)
        lines.append(f'Project("{SOLUTION_GUID_CPP}") = "{display}", "{display}\\{display}.vcxproj", "{guid}"')
        lines.append("EndProject")
    lines.extend(["Global", "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution", "\t\tDebug|x64 = Debug|x64", "\t\tRelease|x64 = Release|x64", "\tEndGlobalSection", "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution"])
    for sample in SAMPLES:
        guid = project_guid(sample)
        lines.extend([f"\t\t{guid}.Debug|x64.ActiveCfg = Debug|x64", f"\t\t{guid}.Debug|x64.Build.0 = Debug|x64", f"\t\t{guid}.Release|x64.ActiveCfg = Release|x64", f"\t\t{guid}.Release|x64.Build.0 = Release|x64"])
    lines.extend(["\tEndGlobalSection", "\tGlobalSection(SolutionProperties) = preSolution", "\t\tHideSolutionNode = FALSE", "\tEndGlobalSection", "EndGlobal", ""])
    return "\n".join(lines)


def roadmap_file() -> str:
    rows = "\n".join(f"| {n} | `{name}` | {goal} | {status} |" for n, name, goal, status in SAMPLES)
    return f'''# Vulkan Learning Roadmap

This roadmap mirrors the scale of the DirectX 11 and DirectX 12 practice sets while using Vulkan-specific ordering: instance and surface first, then swapchain and render passes, followed by buffers, descriptors, images, synchronization, render passes, compute, and renderer structure.

## Learning Principles

- One sample, one primary concept.
- Keep Vulkan ownership explicit in `main.cpp` until a concept needs to move into `LearningStage.h`.
- Put sample-specific resources and render hooks in `LearningStage.h`.
- Prefer deterministic procedural data before asset-loading samples.
- Every sample must have a visible result that can be checked quickly.
- Every sample must build independently and also belong to `VulkanLearning.sln`.

## Full Sequence

| No. | Sample | Goal | Status |
|---|---|---|---|
{rows}

## Implementation Strategy

`01. VulkanBasic` is implemented as the foundation. Later projects start as independent Vulkan clear/present smoke tests with the full folder and documentation contract. Promote each scaffold to `Implemented` by adding only the lesson-specific Vulkan resources and draw/dispatch work described in its README.

## Next Implementation Pass

Start with `02. HelloTriangle`:

- compile GLSL placeholders to SPIR-V or replace them with finalized shader files
- create shader modules, pipeline layout, and graphics pipeline
- replace the clear-only render hook with a triangle draw
- update `ROADMAP.md` and `WORKSPACE_CONTEXT.md`
'''


def workspace_context_file() -> str:
    sample_lines = "\n".join(f"- `{folder(sample)}`: {sample[3]}" for sample in SAMPLES)
    implemented = sum(1 for sample in SAMPLES if sample[3] == "Implemented")
    scaffold = len(SAMPLES) - implemented
    return f'''# Vulkan Practice Workspace Context

## Purpose

This workspace is a Vulkan learning repository built around small, repeatable Visual C++ sample projects. It reflects the existing DirectX 11 and DirectX 12 practice style: numbered samples, one focused concept per sample, independent project files, and explicit markdown context.

## Current State

- Root solution: `VulkanLearning.sln`
- Current API focus: Vulkan on Win32
- Current platform: Windows, x64
- Current Visual Studio toolset: `v145`
- Required SDK: Vulkan SDK available through `VULKAN_SDK`
- Project count: {len(SAMPLES)}
- Implemented sample count: {implemented}
- Scaffolded sample count: {scaffold}

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
    SampleName.vert.glsl
    SampleName.frag.glsl
  assets/
    README.md
  README.md
```

## Sample Contract

`main.cpp` owns the application shell:

- Win32 window creation
- Vulkan instance, surface, device, swapchain, image views, render pass, and framebuffers
- command pool and command buffer lifecycle
- acquire, submit, present, and synchronization
- calls to the stage hooks

`LearningStage.h` owns only sample-specific code:

- stage resources
- setup/update/render/cleanup hooks
- comments around the one new concept introduced by the sample

## Build Notes

Build the whole workspace:

```powershell
& "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe" ".\\VulkanLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Generated build outputs are placed under each sample's `bin/` and `obj/` directories and are ignored by git. Each project expects `$(VULKAN_SDK)\\Include` and `$(VULKAN_SDK)\\Lib`.

## Expansion Rules

- Add each new sample project to `VulkanLearning.sln`.
- Keep each sample independently buildable from its `.vcxproj`.
- Keep one primary learning goal per sample.
- Prefer procedural data for early samples.
- Use GLSL plus SPIR-V compilation when shaders become the primary lesson.
- Document placeholders explicitly in both code and README files.
'''


def root_readme() -> str:
    return '''# Vulkan Learning Samples

Vulkan을 단계적으로 익히기 위한 Visual Studio C++ 샘플 모음이다. 기존 `practice-DirectX11`, `practice-DirectX12`와 같은 번호형 실습 구조를 유지하되 Vulkan의 명시적 객체 수명, 동기화, 디스크립터, 이미지 레이아웃 모델에 맞춰 커리큘럼을 재배치했다.

## Quick Start

1. Vulkan SDK를 설치하고 `VULKAN_SDK` 환경 변수가 설정되어 있는지 확인한다.
2. Visual Studio에서 `VulkanLearning.sln`을 연다.
3. 원하는 번호의 프로젝트를 시작 프로젝트로 설정하고 실행한다.

명령줄 빌드:

```powershell
& "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe" ".\\VulkanLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

## Structure

각 샘플은 독립 `.vcxproj`로 빌드되며 `main.cpp`는 공통 Vulkan 애플리케이션 셸을, `LearningStage.h`는 해당 실습의 고유 리소스와 렌더 훅을 담당한다.

자세한 순서는 `ROADMAP.md`, 현재 작업 컨텍스트는 `WORKSPACE_CONTEXT.md`를 기준으로 한다.
'''


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(text.replace("\n", "\r\n").encode("utf-8"))


def main() -> None:
    for sample in SAMPLES:
        display = folder(sample)
        sample_dir = ROOT / display
        class_name = f"Vulkan{sample[0]}{root_namespace(sample[1])}"
        write_text(sample_dir / f"{display}.vcxproj", project_file(sample))
        write_text(sample_dir / f"{display}.vcxproj.filters", filters_file(sample))
        main_cpp = MAIN_TEMPLATE.replace("{class_name}", class_name).replace("{display_name}", display)
        write_text(sample_dir / "main.cpp", main_cpp)
        write_text(sample_dir / "LearningStage.h", stage_header(sample))
        write_text(sample_dir / "README.md", readme_file(sample))
        write_text(sample_dir / "shaders" / "README.md", shaders_readme(sample))
        write_text(sample_dir / "shaders" / f"{sample[1]}.vert.glsl", shader_file(sample, "vert"))
        write_text(sample_dir / "shaders" / f"{sample[1]}.frag.glsl", shader_file(sample, "frag"))
        write_text(sample_dir / "assets" / "README.md", assets_readme(sample))

    write_text(ROOT / "VulkanLearning.sln", solution_file())
    write_text(ROOT / "ROADMAP.md", roadmap_file())
    write_text(ROOT / "WORKSPACE_CONTEXT.md", workspace_context_file())
    write_text(ROOT / "README.md", root_readme())


if __name__ == "__main__":
    main()
