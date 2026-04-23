#pragma once
#include "Struct.h"


class LightManager
{
    LightManager() = default;
    static LightManager* instance;

public:
    static LightManager* GetLM();

    bool RegisterLight(LightData* light);
    void UnregisterLight(LightData* light);

    // Garder factory pour compatibilité
    //LightData CreatePointLight(DirectX::XMFLOAT3 const& position, DirectX::XMFLOAT4 const& color, float range, float intensity, float strength);

   // LightData CreateSpotLight(DirectX::XMFLOAT3 const& position, DirectX::XMFLOAT3 const& direction, DirectX::XMFLOAT4 const& color, float spotAngle, float penumbraAngle, float range, float intensity, float strength);

    void CreateBuffer(ID3D12Device* device);
    void UpdateLightCB();

    ID3D12Resource* GetLightCB();

private:
    std::vector<LightData*> m_registeredLights;
    ID3D12Resource* m_pLightCB = nullptr;
    uint8_t* m_MappedLightCB = nullptr;
};