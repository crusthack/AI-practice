#include <Windows.h>
#include <rpc.h>
#include <strsafe.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    RPC_WSTR bindingString{};
    RPC_BINDING_HANDLE binding{};
    RPC_STATUS status = RpcStringBindingComposeW(nullptr, reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(L"ncalrpc")), nullptr, reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(L"Win32RoadmapEndpoint")), nullptr, &bindingString);
    if (status == RPC_S_OK)
        status = RpcBindingFromStringBindingW(bindingString, &binding);

    wchar_t msg[512]{};
    StringCchPrintfW(msg, ARRAYSIZE(msg), L"21_DCOM_RPC\r\nRpcStringBindingCompose/RpcBindingFromStringBinding status: %lu\r\nBinding string: %s", status, bindingString ? reinterpret_cast<wchar_t*>(bindingString) : L"(null)");
    if (binding) RpcBindingFree(&binding);
    if (bindingString) RpcStringFreeW(&bindingString);
    MessageBoxW(nullptr, msg, L"DCOM RPC", MB_OK);
    return 0;
}
