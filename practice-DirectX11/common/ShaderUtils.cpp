#include "ShaderUtils.h"

#include <windows.h>

HRESULT CompileShaderFromFile(
    const wchar_t* fileName,
    const char* entryPoint,
    const char* shaderModel,
    Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        fileName,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flags,
        0,
        &shaderBlob,
        &errorBlob);

    if (FAILED(hr) && errorBlob)
        OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));

    return hr;
}
