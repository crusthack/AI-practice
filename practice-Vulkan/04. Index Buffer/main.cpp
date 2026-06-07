#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#include <windows.h>

#include <algorithm>
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
    VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
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
    bool ValidationEnabled = false;
};

void ThrowIfFailed(VkResult result, const char* message)
{
    if (result < 0)
    {
        throw std::runtime_error(message);
    }
}

uint32_t Clamp(uint32_t value, uint32_t low, uint32_t high)
{
    return std::max(low, std::min(high, value));
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData)
{
    (void)severity;
    (void)type;
    (void)userData;

    OutputDebugStringA("Vulkan validation: ");
    OutputDebugStringA(callbackData->pMessage);
    OutputDebugStringA("\n");
    return VK_FALSE;
}

bool IsValidationLayerAvailable()
{
    uint32_t layerCount = 0;
    ThrowIfFailed(vkEnumerateInstanceLayerProperties(&layerCount, nullptr), "vkEnumerateInstanceLayerProperties failed.");
    std::vector<VkLayerProperties> layers(layerCount);
    ThrowIfFailed(vkEnumerateInstanceLayerProperties(&layerCount, layers.data()), "vkEnumerateInstanceLayerProperties failed.");

    for (const auto& layer : layers)
    {
        if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
        {
            return true;
        }
    }

    return false;
}

void FillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

void CreateDebugMessenger(VulkanContext& vk)
{
    if (!vk.ValidationEnabled)
    {
        return;
    }

    auto createMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(vk.Instance, "vkCreateDebugUtilsMessengerEXT"));
    if (!createMessenger)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    FillDebugMessengerCreateInfo(createInfo);
    ThrowIfFailed(createMessenger(vk.Instance, &createInfo, nullptr, &vk.DebugMessenger), "vkCreateDebugUtilsMessengerEXT failed.");
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
    const wchar_t* className = L"Vulkan04IndexBufferWindowClass";

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
        L"04. Index Buffer",
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
    ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr), "vkGetPhysicalDeviceSurfaceFormatsKHR failed.");
    ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr), "vkGetPhysicalDeviceSurfacePresentModesKHR failed.");
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
        ThrowIfFailed(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr), "vkEnumerateDeviceExtensionProperties failed.");
        std::vector<VkExtensionProperties> extensions(extensionCount);
        ThrowIfFailed(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data()), "vkEnumerateDeviceExtensionProperties failed.");

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
    ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr), "vkGetPhysicalDeviceSurfaceFormatsKHR failed.");
    std::vector<VkSurfaceFormatKHR> formats(count);
    ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data()), "vkGetPhysicalDeviceSurfaceFormatsKHR failed.");

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
    ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr), "vkGetPhysicalDeviceSurfacePresentModesKHR failed.");
    std::vector<VkPresentModeKHR> modes(count);
    ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data()), "vkGetPhysicalDeviceSurfacePresentModesKHR failed.");

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
    ThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities), "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed.");
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }

    VkExtent2D extent = { WindowWidth, WindowHeight };
    extent.width = Clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = Clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return extent;
}

void InitVulkan(VulkanContext& vk)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "04. Index Buffer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "VulkanLearning";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    vk.ValidationEnabled = IsValidationLayerAvailable();

    std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
    std::vector<const char*> validationLayers;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (vk.ValidationEnabled)
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        validationLayers.push_back("VK_LAYER_KHRONOS_validation");
        FillDebugMessengerCreateInfo(debugCreateInfo);
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = validationLayers.empty() ? nullptr : validationLayers.data();
    instanceCreateInfo.pNext = vk.ValidationEnabled ? &debugCreateInfo : nullptr;
    ThrowIfFailed(vkCreateInstance(&instanceCreateInfo, nullptr, &vk.Instance), "vkCreateInstance failed.");
    CreateDebugMessenger(vk);

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
    ThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.PhysicalDevice, vk.Surface, &capabilities), "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed.");
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

    ThrowIfFailed(vkGetSwapchainImagesKHR(vk.Device, vk.Swapchain, &imageCount, nullptr), "vkGetSwapchainImagesKHR failed.");
    vk.SwapchainImages.resize(imageCount);
    ThrowIfFailed(vkGetSwapchainImagesKHR(vk.Device, vk.Swapchain, &imageCount, vk.SwapchainImages.data()), "vkGetSwapchainImagesKHR failed.");

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
    if (vk.DebugMessenger)
    {
        auto destroyMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(vk.Instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (destroyMessenger)
        {
            destroyMessenger(vk.Instance, vk.DebugMessenger, nullptr);
        }
    }
    if (vk.Instance) vkDestroyInstance(vk.Instance, nullptr);
}

int Run(HINSTANCE instance, int showCommand)
{
    InitWindow(instance, showCommand);

    VulkanContext vk = {};
    LearningStageState stage = {};
    try
    {
        InitVulkan(vk);
        ApplyStageSpecificSetup(stage, vk.PhysicalDevice, vk.Device, vk.RenderPass, vk.SwapchainExtent, vk.CommandPool, vk.GraphicsQueue);

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
    catch (...)
    {
        ApplyStageSpecificCleanup(stage);
        Cleanup(vk);
        throw;
    }
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
        MessageBoxW(nullptr, message.c_str(), L"04. Index Buffer Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}
