#include "pch.h"
//#include "TextureHelper.h"
//
//inline bool TextureHelper::IsTextureLoaded(const Texture& tex)
//{
//    return tex.Resource != nullptr;
//}
//
//inline bool TextureHelper::FormatHasAlpha(DXGI_FORMAT fmt)
//{
//    switch (fmt)
//    {
//    case DXGI_FORMAT_R8G8B8A8_UNORM:
//    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
//    case DXGI_FORMAT_B8G8R8A8_UNORM:
//    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
//    case DXGI_FORMAT_R32G32B32A32_FLOAT:
//    case DXGI_FORMAT_BC3_UNORM:
//    case DXGI_FORMAT_BC3_UNORM_SRGB:
//    case DXGI_FORMAT_BC7_UNORM:
//    case DXGI_FORMAT_BC7_UNORM_SRGB:
//        return true;
//    default:
//        return false;
//    }
//}
//
//inline bool TextureHelper::GetTextureDesc(const Texture& tex, D3D12_RESOURCE_DESC& outDesc)
//{
//    if (!IsTextureLoaded(tex)) return false;
//    outDesc = tex.Resource->GetDesc();
//    return true;
//}
//
//inline bool TextureHelper::LooksLikeSprite(const Texture& tex)
//{
//    D3D12_RESOURCE_DESC desc;
//    if (!GetTextureDesc(tex, desc)) return false;
//
//    if (desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D) return false;
//
//    return FormatHasAlpha(desc.Format);
//}
