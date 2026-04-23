#pragma once
#include "pch.h"

#define MAX_LIGHTS 16

class Geometry;

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT4 color;
    XMFLOAT3 Normal;
    XMFLOAT2 uv;
};

struct MaterialConstants
{
    XMFLOAT4 color;
    float roughness;
    float metallic;

    float isUI;        // 0 = 3D, 1 = UI
    float padding[1];  // align 16 bytes
};

struct Textures
{
    std::string Name;

    std::wstring Filename;

    ID3D12Resource* Resource = nullptr;
    ID3D12Resource* UploadHeap = nullptr;
};

enum class LightType : int
{
    NONE = -1,
    DIRECTIONAL,
    POINT,
    SPOT
};

struct LightData
{
    LightType type;
    DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 direction = { 0.0f, 0.0f, 1.0f };
    DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 strength = { 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT4 rimLightColor = { 1,1,1,1 };
    float range = 10.0f;
    float intensity = 1.0f;
    float spotAngle;
    float penumbraAngle;
    float rimLightIntensity;
    int id = -1;
};

struct LightPassData
{
    LightData light[MAX_LIGHTS];

    float ambientIntensity = 0.5f;
    float padding2 = 0;
};

struct LightCBData
{
    DirectX::XMFLOAT4 LightPosRange[MAX_LIGHTS];
    DirectX::XMFLOAT4 LightColorIntensity[MAX_LIGHTS];
    DirectX::XMFLOAT4 LightStrengthPad[MAX_LIGHTS];
    DirectX::XMFLOAT4 LightDirType[MAX_LIGHTS];
    DirectX::XMFLOAT4 LightSpotAngles[MAX_LIGHTS];
    int LightCount;
    float padding[3];
};