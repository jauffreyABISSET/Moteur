#include "pch.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
    XMStoreFloat4x4(&mView, XMMatrixIdentity());
    XMStoreFloat4x4(&mProj, XMMatrixIdentity());
}

float Camera::GetFovX()const
{
    float halfWidth = 0.5f * GetNearWindowWidth();
    return 2.0f * atan(halfWidth / mNearZ);
}
float Camera::GetNearWindowWidth()const
{
    return mAspect * mNearWindowHeight;
}
float Camera::GetNearWindowHeight()const
{
    return mNearWindowHeight;
}
float Camera::GetFarWindowWidth()const
{
    return mAspect * mFarWindowHeight;
}
float Camera::GetFarWindowHeight()const
{
    return mFarWindowHeight;
}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
    mFovY = fovY;
    mAspect = aspect;
    mNearZ = zn;
    mFarZ = zf;
    mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f * mFovY);
    mFarWindowHeight = 2.0f * mFarZ * tanf(0.5f * mFovY);
    XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
    XMStoreFloat4x4(&mProj, P);
}

void Camera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR up)
{
    XMVECTOR L = XMVector3Normalize(target - pos);
    XMVECTOR R = XMVector3Normalize(XMVector3Cross(up, L));
    XMVECTOR U = XMVector3Cross(L, R);

	m_transform.SetPosition(pos);
	m_transform.SetRight(R);
	m_transform.SetUp(U);
    m_transform.SetLook(L);
	mViewDirty = true;
}

void Camera::LookAt(XMFLOAT3 target)
{
    XMFLOAT3 pos = m_transform.GetWorldPosition();
    XMVECTOR posV = XMLoadFloat3(&pos);
    XMVECTOR targetV = XMLoadFloat3(&target);
    XMVECTOR upV = m_transform.GetUp();

    XMVECTOR L = XMVector3Normalize(targetV - posV);
    XMVECTOR R = XMVector3Normalize(XMVector3Cross(upV, L));
    XMVECTOR U = XMVector3Cross(L, R);

    m_transform.SetPosition(posV);
    m_transform.SetRight(R);
    m_transform.SetUp(U);
    m_transform.SetLook(L);

    mViewDirty = true;
}

void Camera::UpdateViewMatrix()
{
    if (!mViewDirty && !m_transform.IsDirty()) 
        return;

    XMVECTOR R = m_transform.GetRight();
    XMVECTOR U = m_transform.GetUp();
    XMVECTOR L = m_transform.GetLook();
	XMVECTOR P = m_transform.GetMatrixPosition().r[3];

    // Orthonormalize
    L = XMVector3Normalize(L);
    U = XMVector3Normalize(XMVector3Cross(L, R));
    R = XMVector3Cross(U, L);

    float x = -XMVectorGetX(XMVector3Dot(P, R));
    float y = -XMVectorGetX(XMVector3Dot(P, U));
    float z = -XMVectorGetX(XMVector3Dot(P, L));

    XMFLOAT3 rightF, upF, lookF;
    XMStoreFloat3(&rightF, R);
    XMStoreFloat3(&upF, U);
    XMStoreFloat3(&lookF, L);

    mView(0, 0) = rightF.x;
    mView(1, 0) = rightF.y;
    mView(2, 0) = rightF.z;
    mView(3, 0) = x;

    mView(0, 1) = upF.x;
    mView(1, 1) = upF.y;
    mView(2, 1) = upF.z;
    mView(3, 1) = y;

    mView(0, 2) = lookF.x;
    mView(1, 2) = lookF.y;
    mView(2, 2) = lookF.z;
    mView(3, 2) = z;

    mView(0, 3) = 0.0f;
    mView(1, 3) = 0.0f;
    mView(2, 3) = 0.0f;
    mView(3, 3) = 1.0f;

    // clear dirty flags
    mViewDirty = false;
    m_transform.ClearDirty();
}

XMMATRIX Camera::GetView() const
{
    return XMLoadFloat4x4(&mView);
}

XMMATRIX Camera::GetProj() const
{
    return XMLoadFloat4x4(&mProj);
}

XMFLOAT4X4 Camera::GetView4x4f() const { return mView; }
XMFLOAT4X4 Camera::GetProj4x4f() const { return mProj; }