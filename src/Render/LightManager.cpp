#include "pch.h"
#include "LightManager.h"


LightManager* LightManager::instance = nullptr;

LightManager* LightManager::GetLM()
{
    if (instance == nullptr) {
        instance = new LightManager();
    }
    return instance;
}

bool LightManager::RegisterLight(LightData* light)
{
    if (!light) return false;
    if ((int)m_registeredLights.size() >= MAX_LIGHTS) return false;
    m_registeredLights.push_back(light);

    int assignedId = static_cast<int>(m_registeredLights.size()) - 1;
    light->id = assignedId;

#if defined(_DEBUG) || defined(DEBUG)
    char buf[128];
    sprintf_s(buf, sizeof(buf), "LightManager::RegisterLight - registered, total=%zu id=%d\n", m_registeredLights.size(), light->id);
    OutputDebugStringA(buf);
#endif

    return true;
}

void LightManager::UnregisterLight(LightData* light)
{
    if (!light) return;
    for (size_t i = 0; i < m_registeredLights.size(); ++i)
    {
        if (m_registeredLights[i] == light)
        {
            size_t lastIndex = m_registeredLights.size() - 1;
            if (i != lastIndex)
            {
                m_registeredLights[i] = m_registeredLights.back();
                m_registeredLights[i]->id = static_cast<int>(i);
            }
            m_registeredLights.pop_back();

            light->id = -1;

#if defined(_DEBUG) || defined(DEBUG)
            char buf[128];
            sprintf_s(buf, sizeof(buf), "LightManager::UnregisterLight - removed, total=%zu\n", m_registeredLights.size());
            OutputDebugStringA(buf);
#endif
            return;
        }
    }
}

//LightData LightManager::CreatePointLight(DirectX::XMFLOAT3 const& position, DirectX::XMFLOAT4 const& color, float range, float intensity, float strength)
//{
//    LightData light = {};
//    light.type = LightType::POINT;
//    light.position = position;
//    light.color = color;
//    light.range = range;
//    light.intensity = intensity;
//    light.strength = DirectX::XMFLOAT3(strength, strength, strength);
//    light.rimLightColor = color;
//    light.rimLightIntensity = intensity;
//    return light;
//}

//LightData LightManager::CreateSpotLight(DirectX::XMFLOAT3 const& position, DirectX::XMFLOAT3 const& direction, DirectX::XMFLOAT4 const& color, float spotAngle, float penumbraAngle, float range, float intensity, float strength)
//{
//    LightData light = {};
//    light.type = LightType::SPOT;
//    light.position = position;
//    light.direction = direction;
//    light.color = color;
//    light.spotAngle = spotAngle;
//    light.penumbraAngle = penumbraAngle;
//    light.range = range;
//    light.intensity = intensity;
//    light.strength = DirectX::XMFLOAT3(strength, strength, strength);
//    light.rimLightColor = color;
//    light.rimLightIntensity = 0.4f;
//    return light;
//}

void LightManager::CreateBuffer(ID3D12Device* device)
{
    if (!device) {
        OutputDebugStringA("LightManager::CreateBuffer - device is null\n");
        return;
    }

    UINT64 byteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(LightCBData));

    CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);

    if (m_pLightCB)
    {
        m_pLightCB->Unmap(0, nullptr);
        m_MappedLightCB = nullptr;
        m_pLightCB->Release();
        m_pLightCB = nullptr;
    }

    ThrowIfFailed(device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pLightCB)
    ));

    ThrowIfFailed(m_pLightCB->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedLightCB)));

#if defined(_DEBUG) || defined(DEBUG)
    OutputDebugStringA("LightManager::CreateBuffer - created and mapped Light CB\n");
#endif
}

void LightManager::UpdateLightCB()
{
    if (!m_pLightCB || !m_MappedLightCB) return;

    LightCBData cbData = {};
    cbData.LightCount = static_cast<int>(std::min<size_t>(m_registeredLights.size(), MAX_LIGHTS));

    for (int i = 0; i < cbData.LightCount; i++)
    {
        const LightData* L = m_registeredLights[i];
        if (!L) continue;

        cbData.LightPosRange[i] = XMFLOAT4(L->position.x, L->position.y, L->position.z, L->range);
        cbData.LightColorIntensity[i] = XMFLOAT4(L->color.x, L->color.y, L->color.z, L->intensity);
        cbData.LightStrengthPad[i] = XMFLOAT4(L->strength.x, L->strength.y, L->strength.z, L->rimLightIntensity);

        XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&L->direction));
        XMFLOAT3 nDir;
        XMStoreFloat3(&nDir, dir);
        cbData.LightDirType[i] = XMFLOAT4(nDir.x, nDir.y, nDir.z, (float)L->type);

        // stocker cosines pour le shader
        float cosInner = cosf(XMConvertToRadians(L->spotAngle));    // angle plein
        float cosOuter = cosf(XMConvertToRadians(L->penumbraAngle));// bord flou
        cbData.LightSpotAngles[i] = XMFLOAT4(cosInner, cosOuter, 0.f, 0.f);
    }

    memcpy(m_MappedLightCB, &cbData, sizeof(LightCBData));
}

ID3D12Resource* LightManager::GetLightCB()
{
    return m_pLightCB;
}