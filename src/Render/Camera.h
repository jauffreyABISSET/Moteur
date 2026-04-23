#pragma once
#include "Core/FrameWork.h"

using namespace DirectX;

class Transform;

class Camera
{
public:
    Camera();

    // Frustum
    float GetFovX() const;
    float GetNearWindowWidth() const;
    float GetNearWindowHeight() const;
    float GetFarWindowWidth() const;
    float GetFarWindowHeight() const;

    void SetLens(float fovY, float aspect, float zn, float zf);

    // LookAt
    void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR up);
	void LookAt(XMFLOAT3 target);

    void UpdateViewMatrix();

    // Matrices
    XMMATRIX GetView() const;
    XMMATRIX GetProj() const;
    XMFLOAT4X4 GetView4x4f() const;
    XMFLOAT4X4 GetProj4x4f() const;

    Transform m_transform;
private:

    // Cache frustum properties.
    float mNearZ = 0.0f;
    float mFarZ = 0.0f;
    float mAspect = 0.0f;
    float mFovY = 0.0f;
    float mNearWindowHeight = 0.0f;
    float mFarWindowHeight = 0.0f;

    bool mViewDirty = true;

    // Cache View/Proj matrices.
    DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();
};