#include <windows.h>
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
{
constexpr uint32_t FrameCount = 2;
constexpr uint32_t WindowWidth = 1280;
constexpr uint32_t WindowHeight = 720;

HWND g_hwnd = nullptr;

struct Dx12Context
{
    ComPtr<IDXGIFactory4> Factory;
    ComPtr<ID3D12Device> Device;
    ComPtr<ID3D12CommandQueue> CommandQueue;
    ComPtr<IDXGISwapChain3> SwapChain;
    ComPtr<ID3D12DescriptorHeap> RtvHeap;
    ComPtr<ID3D12Resource> RenderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> CommandAllocators[FrameCount];
    ComPtr<ID3D12GraphicsCommandList> CommandList;
    ComPtr<ID3D12Fence> Fence;
    uint64_t FenceValues[FrameCount] = {};
    HANDLE FenceEvent = nullptr;
    uint32_t RtvDescriptorSize = 0;
    uint32_t FrameIndex = 0;
};

void ThrowIfFailed(HRESULT hr, const char* message)
{
    if (FAILED(hr))
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
    const wchar_t* className = L"Dx1201Dx12BasicWindowClass";

    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.lpszClassName = className;
    ThrowIfFailed(RegisterClassExW(&windowClass) ? S_OK : E_FAIL, "RegisterClassExW failed.");

    RECT rect = { 0, 0, static_cast<LONG>(WindowWidth), static_cast<LONG>(WindowHeight) };
    ThrowIfFailed(AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE) ? S_OK : E_FAIL, "AdjustWindowRect failed.");

    g_hwnd = CreateWindowExW(
        0,
        className,
        L"01. Dx12Basic",
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
}

ComPtr<IDXGIAdapter1> ChooseHardwareAdapter(IDXGIFactory4* factory)
{
    ComPtr<IDXGIAdapter1> adapter;

    for (UINT index = 0; factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND; ++index)
    {
        DXGI_ADAPTER_DESC1 desc = {};
        adapter->GetDesc1(&desc);

        if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
        {
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
        {
            return adapter;
        }
    }

    return nullptr;
}

void InitGraphicsDevice(Dx12Context& dx)
{
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
    }
#endif

    UINT factoryFlags = 0;
#if defined(_DEBUG)
    factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
    ThrowIfFailed(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&dx.Factory)), "CreateDXGIFactory2 failed.");

    ComPtr<IDXGIAdapter1> adapter = ChooseHardwareAdapter(dx.Factory.Get());
    ThrowIfFailed(adapter ? S_OK : E_FAIL, "No suitable DX12 hardware adapter found.");

    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dx.Device)), "D3D12CreateDevice failed.");

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(dx.Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&dx.CommandQueue)), "CreateCommandQueue failed.");

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
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
}

void CreateCommonResources(Dx12Context& dx)
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(dx.Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&dx.RtvHeap)), "CreateDescriptorHeap failed.");

    dx.RtvDescriptorSize = dx.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dx.RtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (uint32_t i = 0; i < FrameCount; ++i)
    {
        ThrowIfFailed(dx.SwapChain->GetBuffer(i, IID_PPV_ARGS(&dx.RenderTargets[i])), "GetBuffer failed.");
        dx.Device->CreateRenderTargetView(dx.RenderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += dx.RtvDescriptorSize;

        ThrowIfFailed(
            dx.Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&dx.CommandAllocators[i])),
            "CreateCommandAllocator failed.");
    }

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
}

D3D12_CPU_DESCRIPTOR_HANDLE CurrentRtvHandle(const Dx12Context& dx)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = dx.RtvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(dx.FrameIndex) * dx.RtvDescriptorSize;
    return handle;
}

void BeginFrame(Dx12Context& dx)
{
    ThrowIfFailed(dx.CommandAllocators[dx.FrameIndex]->Reset(), "CommandAllocator reset failed.");
    ThrowIfFailed(dx.CommandList->Reset(dx.CommandAllocators[dx.FrameIndex].Get(), nullptr), "CommandList reset failed.");

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = dx.RenderTargets[dx.FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    dx.CommandList->ResourceBarrier(1, &barrier);

    const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CurrentRtvHandle(dx);
    dx.CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
}

void Render(Dx12Context& dx, LearningStageState& stage)
{
    BeginFrame(dx);

    LearningStageRenderContext stageContext = {};
    stageContext.CommandList = dx.CommandList.Get();
    stageContext.RenderTargetView = CurrentRtvHandle(dx);
    ApplyStageSpecificRender(stage, stageContext);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = dx.RenderTargets[dx.FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    dx.CommandList->ResourceBarrier(1, &barrier);

    ThrowIfFailed(dx.CommandList->Close(), "CommandList close failed.");

    ID3D12CommandList* commandLists[] = { dx.CommandList.Get() };
    dx.CommandQueue->ExecuteCommandLists(1, commandLists);
}

void MoveToNextFrame(Dx12Context& dx)
{
    const uint64_t currentFenceValue = dx.FenceValues[dx.FrameIndex];
    ThrowIfFailed(dx.CommandQueue->Signal(dx.Fence.Get(), currentFenceValue), "Fence signal failed.");

    dx.FrameIndex = dx.SwapChain->GetCurrentBackBufferIndex();

    if (dx.Fence->GetCompletedValue() < dx.FenceValues[dx.FrameIndex])
    {
        ThrowIfFailed(dx.Fence->SetEventOnCompletion(dx.FenceValues[dx.FrameIndex], dx.FenceEvent), "SetEventOnCompletion failed.");
        WaitForSingleObject(dx.FenceEvent, INFINITE);
    }

    dx.FenceValues[dx.FrameIndex] = currentFenceValue + 1;
}

void WaitForGpu(Dx12Context& dx)
{
    ThrowIfFailed(dx.CommandQueue->Signal(dx.Fence.Get(), dx.FenceValues[dx.FrameIndex]), "Fence signal failed.");
    ThrowIfFailed(dx.Fence->SetEventOnCompletion(dx.FenceValues[dx.FrameIndex], dx.FenceEvent), "SetEventOnCompletion failed.");
    WaitForSingleObject(dx.FenceEvent, INFINITE);
    ++dx.FenceValues[dx.FrameIndex];
}

void EndFrame(Dx12Context& dx)
{
    ThrowIfFailed(dx.SwapChain->Present(1, 0), "Present failed.");
    MoveToNextFrame(dx);
}

void CleanupCommonResources(Dx12Context& dx)
{
    WaitForGpu(dx);

    if (dx.FenceEvent)
    {
        CloseHandle(dx.FenceEvent);
        dx.FenceEvent = nullptr;
    }
}

int Run(HINSTANCE instance, int showCommand)
{
    InitWindow(instance, showCommand);

    Dx12Context dx = {};
    LearningStageState stage = {};

    InitGraphicsDevice(dx);
    CreateCommonResources(dx);
    ApplyStageSpecificSetup(stage, dx.Device.Get());

    auto startTime = std::chrono::steady_clock::now();
    double prevTime = 0.0;
    uint32_t fpsFrameCount = 0;
    double fpsAccum = 0.0;

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        const auto now = std::chrono::steady_clock::now();
        const double timeSeconds = std::chrono::duration<double>(now - startTime).count();
        const double deltaTime = timeSeconds - prevTime;
        prevTime = timeSeconds;

        fpsAccum += deltaTime;
        ++fpsFrameCount;
        if (fpsAccum >= 1.0)
        {
            const int fps = static_cast<int>(fpsFrameCount / fpsAccum + 0.5);
            const int ms  = static_cast<int>(1000.0 * fpsAccum / fpsFrameCount + 0.5);
            wchar_t title[64];
            wsprintf(title, L"01. Dx12 Basic  |  %d fps  |  %d ms/frame", fps, ms);
            SetWindowTextW(g_hwnd, title);
            fpsFrameCount = 0;
            fpsAccum = 0.0;
        }

        UpdateStageSpecificDemo(stage, timeSeconds);
        Render(dx, stage);
        EndFrame(dx);
    }

    ApplyStageSpecificCleanup(stage);
    CleanupCommonResources(dx);
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
        MessageBoxW(nullptr, message.c_str(), L"01. Dx12Basic Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}
