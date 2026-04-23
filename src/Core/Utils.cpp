#include "pch.h"
#include "Utils.h"
#include <comdef.h>
#include <fstream>

bool d3dUtil::IsKeyDown(int vkeyCode)
{
    return (GetAsyncKeyState(vkeyCode) & 0x8000) != 0;
}

ID3DBlob* d3dUtil::LoadBinary(const std::wstring& filename)
{
    std::ifstream fin(filename, std::ios::binary);

    fin.seekg(0, std::ios_base::end);
    std::ifstream::pos_type size = (int)fin.tellg();
    fin.seekg(0, std::ios_base::beg);

    ID3DBlob* blob;
    ThrowIfFailed(D3DCreateBlob(size, &blob));

    fin.read((char*)blob->GetBufferPointer(), size);
    fin.close();

    return blob;
}

ID3D12Resource* d3dUtil::CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    ID3D12Resource& uploadBuffer)
{
    ID3D12Resource* defaultBuffer;

    CD3DX12_HEAP_PROPERTIES heap_Properties511615 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC ressource_Desc125 = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    // Create the actual default buffer resource.
    ThrowIfFailed(device->CreateCommittedResource(
        &heap_Properties511615,
        D3D12_HEAP_FLAG_NONE,
        &ressource_Desc125,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&defaultBuffer)));

    CD3DX12_HEAP_PROPERTIES heap_Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC ressource_Desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    // In order to copy CPU memory data into our default buffer, we need to create
    // an intermediate upload heap. 
    ThrowIfFailed(device->CreateCommittedResource(
        &heap_Properties511615,
        D3D12_HEAP_FLAG_NONE,
        &ressource_Desc125,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&defaultBuffer)));

    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        defaultBuffer,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COPY_DEST);

    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        defaultBuffer,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_GENERIC_READ);

    cmdList->ResourceBarrier(1, &barrier);
    UpdateSubresources<1>(cmdList, defaultBuffer, &uploadBuffer, 0, 0, 1, &subResourceData);
    cmdList->ResourceBarrier(1, &barrier2);

    return defaultBuffer;
}

ID3DBlob* d3dUtil::CompileShader(
    const std::wstring& filename,
    const D3D_SHADER_MACRO* defines,
    const std::string& entrypoint,
    const std::string& target)
{
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = S_OK;

    ID3DBlob* byteCode = nullptr;
    ID3DBlob* errors;
    hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

    if (errors != nullptr)
        OutputDebugStringA((char*)errors->GetBufferPointer());

    ThrowIfFailed(hr);

    return byteCode;
}

std::wstring DxException::ToString()const
{
    // Get the string description of the error code.
    _com_error err(ErrorCode);
    std::wstring msg = err.ErrorMessage();

    return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}

std::string Utils::ReadAll(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}