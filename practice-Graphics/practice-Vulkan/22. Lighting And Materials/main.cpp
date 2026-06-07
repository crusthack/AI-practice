#define VK_USE_PLATFORM_WIN32_KHR
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
    const wchar_t* className = L"Vulkan22LightingAndMaterialsWindowClass";

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
        L"22. Lighting And Materials",
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
    appInfo.pApplicationName = "22. Lighting And Materials";
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
        MessageBoxW(nullptr, message.c_str(), L"22. Lighting And Materials Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}
