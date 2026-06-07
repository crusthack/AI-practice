#pragma once

#include <d3dcompiler.h>
#include <string>
#include <wrl/client.h>

HRESULT CompileShaderFromFile(
    const wchar_t* fileName,
    const char* entryPoint,
    const char* shaderModel,
    Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob);
