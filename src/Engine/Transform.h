#pragma once
#include "Component.h"
#include <vector>
#include "Engine/pch.h"
#include <list>
#include <cmath>

enum Direction {
    Forward,
    Backward,
    Right,
    Left,
    Up,
    Down
};

class Transform
{
private:
    XMFLOAT3 m_pos{ 0,0,0 };
    XMFLOAT3 m_offset{ 0,0,0 };
    XMFLOAT3 m_lastPos{ 0,0,0 };
    XMFLOAT3 m_rot{ 0,0,0 };
    XMFLOAT3 m_scale{ 1,1,1 };
    XMFLOAT4 m_quat{ 0,0,0,1 };

    XMMATRIX m_worldMatrix{};

    XMFLOAT3 m_oscOrigin{ 0,0,0 };
    float m_orbitAngle = 0.0f;
    bool m_oscInitialized = false;
    float   m_oscTime = 0.0f;
    float   m_prevOsc = 0.0f;

    XMFLOAT3 mRight{ 1.f, 0.f, 0.f };
    XMFLOAT3 mUp{ 0.f, 1.f, 0.f };
    XMFLOAT3 mLook{ 0.f, 0.f, 1.f };

    void SaveLastPosition();
    bool    m_dirty = true;
public:
    Transform();

    void Pitch(float angle);
    void RotateY(float angle);

    XMFLOAT4 EulerDegreesToQuaternion(float pitchDeg, float yawDeg, float rollDeg);

    XMFLOAT3 GetLocalPosition() const;
    XMFLOAT3 GetWorldPosition() const;
    XMFLOAT3 GetLastLocalPosition() const;
    const XMFLOAT3& GetLastWorldPosition() const;

    void SetLastLocalPosition(const XMFLOAT3& pos);
    void SetLastWorldPosition(const XMFLOAT3& pos);

    const XMMATRIX GetMatrixPosition() const;
    void SetPosition(const XMFLOAT3& pos);
    void SetPosition(float& x, float& y, float& z);
    void SetPosition(FXMVECTOR pos);

    const XMFLOAT3& GetOffset() const;
    void SetOffset(const XMFLOAT3& offset);
    void SetOffset(float x, float y, float z);
    void SetOffsetFromDirectionPoint(const XMFLOAT3& direction, const XMFLOAT3& pivot, float distance, bool directionIsLocal = false);

    const XMFLOAT3& GetRotation() const;
    const XMMATRIX GetMatrixRotation() const;
    void SetRotation(const XMFLOAT3& r);
    void SetRotation(float x, float y, float z);
    void SetRotationDegrees(float xDeg, float yDeg, float zDeg);
    void SetRotationDegrees(XMFLOAT3 rot);

	XMFLOAT4 GetQuat() const;
	void SetQuat(const XMFLOAT4& q);

    const XMFLOAT4& LocalRotation() const;
    void SetLocalRotation(const XMFLOAT4& q);
    void AddLocalRotation(const XMFLOAT4& q);
    void AddLocalRotation(const XMFLOAT3& eulerDegrees);
    XMFLOAT4 GetLocalRotationQuaternion() const;

    const XMFLOAT3& GetScale() const;
    const XMMATRIX GetMatrixScale() const;

    void SetScale(const XMFLOAT3& s) { m_scale = s; }
    void SetScale(float x, float y, float z);

    void LocalTranslate(Direction dir, float distance, float deltaTime);
    void GoForward(float distance, float deltaTime);
    void GoLeft(float distance, float deltaTime);
    void GoRight(float distance, float deltaTime);
    void GoBackward(float distance, float deltaTime);
    void GoUp(float distance, float deltaTime);
    void GoDown(float distance, float deltaTime);

    XMVECTOR GetRight();
    void SetRight(FXMVECTOR right);
    XMVECTOR GetUp();
	void SetUp(FXMVECTOR up);
    XMVECTOR GetLook();
	void SetLook(FXMVECTOR look);

    void Translate(const XMFLOAT3& delta);
    void Rotate(const XMFLOAT3& delta);

    void GoToPostion(const XMFLOAT3& pos);
    void SlowDow(const XMFLOAT3& pos, float m = 1.f);
    void LookAt(const XMFLOAT3& target);

    void ResetAll();

    void OrbitAroundAxis(const XMFLOAT3& center, const XMFLOAT3& axis, float radius, float angularSpeed, float dt);
    void LocalOscillation(const XMFLOAT3& axis, float amplitude, float frequency, float time);
    void WorldOscillation(const XMFLOAT3& axis, float amplitude, float frequency, float time);

    XMMATRIX GetWorldMatrix() const;

    void SetPositionNoRecord(const XMFLOAT3& pos);
    bool IsDirty() const { return m_dirty; }
    void ClearDirty() { m_dirty = false; }
};