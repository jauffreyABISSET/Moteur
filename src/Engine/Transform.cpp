#include "pch.h"
#include "Transform.h"

Transform::Transform()
{
    m_pos = XMFLOAT3(0.f, 0.f, 0.f);
    m_lastPos = m_pos;
    m_offset = XMFLOAT3(0.f, 0.f, 0.f);
    m_rot = XMFLOAT3(0.f, 0.f, 0.f);
    m_scale = XMFLOAT3(1.f, 1.f, 1.f);
    m_quat = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
    XMVECTOR qv = XMLoadFloat4(&m_quat);
    XMStoreFloat3(&mRight, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), qv));
    XMStoreFloat3(&mUp, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), qv));
    XMStoreFloat3(&mLook, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), qv));
    m_dirty = true;
}

XMVECTOR Transform::GetRight()
{
    return XMLoadFloat3(&mRight); 
}

void Transform::SetRight(XMVECTOR right)
{
    XMStoreFloat3(&mRight, right);
    m_dirty = true;
}

XMVECTOR Transform::GetUp() 
{ 
    return XMLoadFloat3(&mUp); 
}

void Transform::SetUp(XMVECTOR up)
{
    XMStoreFloat3(&mUp, up);
	m_dirty = true;
}

XMVECTOR Transform::GetLook() 
{
    return XMLoadFloat3(&mLook);
}

void Transform::SetLook(XMVECTOR look)
{
    XMStoreFloat3(&mLook, look);
}

void Transform::Pitch(float angle)
{
    XMVECTOR R = XMLoadFloat3(&mRight);
    XMMATRIX Rm = XMMatrixRotationAxis(R, angle);

    XMVECTOR upV = XMLoadFloat3(&mUp);
    XMVECTOR lookV = XMLoadFloat3(&mLook);

    XMStoreFloat3(&mUp, XMVector3TransformNormal(upV, Rm));
    XMStoreFloat3(&mLook, XMVector3TransformNormal(lookV, Rm));

    XMVECTOR Rv = XMLoadFloat3(&mRight);
    XMVECTOR Uv = XMLoadFloat3(&mUp);
    XMVECTOR Lv = XMLoadFloat3(&mLook);
    XMMATRIX rotMat = XMMATRIX(Rv, Uv, Lv, XMVectorSet(0.f, 0.f, 0.f, 1.f));
    XMVECTOR rotQ = XMQuaternionNormalize(XMQuaternionRotationMatrix(rotMat));
    XMStoreFloat4(&m_quat, rotQ);

    m_dirty = true;
}

void Transform::RotateY(float angle)
{
    XMMATRIX R = XMMatrixRotationY(angle);

    XMVECTOR rightV = XMLoadFloat3(&mRight);
    XMVECTOR upV = XMLoadFloat3(&mUp);
    XMVECTOR lookV = XMLoadFloat3(&mLook);

    XMStoreFloat3(&mRight, XMVector3TransformNormal(rightV, R));
    XMStoreFloat3(&mUp, XMVector3TransformNormal(upV, R));
    XMStoreFloat3(&mLook, XMVector3TransformNormal(lookV, R));

    XMVECTOR Rv = XMLoadFloat3(&mRight);
    XMVECTOR Uv = XMLoadFloat3(&mUp);
    XMVECTOR Lv = XMLoadFloat3(&mLook);
    XMMATRIX rotMat = XMMATRIX(Rv, Uv, Lv, XMVectorSet(0.f, 0.f, 0.f, 1.f));
    XMVECTOR rotQ = XMQuaternionNormalize(XMQuaternionRotationMatrix(rotMat));
    XMStoreFloat4(&m_quat, rotQ);

    m_dirty = true;
}

XMFLOAT4 Transform::EulerDegreesToQuaternion(float pitchDeg, float yawDeg, float rollDeg)
{
    XMVECTOR q = XMQuaternionRotationRollPitchYaw(
        XMConvertToRadians(pitchDeg), // pitch = rotation autour de X
        XMConvertToRadians(yawDeg),   // yaw   = rotation autour de Y
        XMConvertToRadians(rollDeg)   // roll  = rotation autour de Z
    );
    XMFLOAT4 out;
    XMStoreFloat4(&out, q);
    return out;
}

XMFLOAT3 Transform::GetLocalPosition() const
{
	return XMFLOAT3{ m_pos.x - m_offset.x, m_pos.y - m_offset.y, m_pos.z - m_offset.z };
}

XMFLOAT3 Transform::GetWorldPosition() const
{
    XMVECTOR posV = XMLoadFloat3(&m_pos);
    XMVECTOR offV = XMLoadFloat3(&m_offset);
    XMVECTOR q = XMLoadFloat4(&m_quat);
    XMVECTOR offRot = XMVector3Rotate(offV, q);
    XMVECTOR world = XMVectorAdd(posV, offRot);
    XMFLOAT3 out; XMStoreFloat3(&out, world);
    return out;
}

XMMATRIX Transform::GetWorldMatrix() const
{
    XMVECTOR posV = XMLoadFloat3(&m_pos);
    XMVECTOR offV = XMLoadFloat3(&m_offset);
    XMVECTOR q = XMLoadFloat4(&m_quat);
    XMVECTOR offRot = XMVector3Rotate(offV, q);
    XMVECTOR worldPos = XMVectorAdd(posV, offRot);
    XMFLOAT3 wp; XMStoreFloat3(&wp, worldPos);

    XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    XMMATRIX R = XMMatrixRotationQuaternion(q);
    XMMATRIX T = XMMatrixTranslation(wp.x, wp.y, wp.z);
    return S * R * T;
}

XMFLOAT3 Transform::GetLastLocalPosition() const
{
    return XMFLOAT3{ m_lastPos.x - m_offset.x, m_lastPos.y - m_offset.y, m_lastPos.z - m_offset.z };
}

const XMFLOAT3& Transform::GetLastWorldPosition() const
{
	return m_lastPos;
}

void Transform::SetLastLocalPosition(const XMFLOAT3& pos)
{
	m_lastPos = XMFLOAT3{ pos.x + m_offset.x, pos.y + m_offset.y, pos.z + m_offset.z };
}

void Transform::SetLastWorldPosition(const XMFLOAT3& pos)
{
	m_lastPos = pos;
}

const XMMATRIX Transform::GetMatrixPosition() const
{
	return XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
}

void Transform::SetPosition(const XMFLOAT3& pos)
{
    SaveLastPosition();
    m_pos = pos;
    m_dirty = true;
}

void Transform::SetPosition(float& x, float& y, float& z)
{
    SaveLastPosition();

    m_pos = XMFLOAT3{ x, y, z };
}

void Transform::SetPosition(DirectX::FXMVECTOR pos)
{
    XMFLOAT3 temp;
    XMStoreFloat3(&temp, pos);
    SetPosition(temp);
}

const XMFLOAT3& Transform::GetOffset() const
{
	return m_offset;
}

void Transform::SetOffset(const XMFLOAT3& offset)
{
    SaveLastPosition();
    m_offset = offset;
    m_dirty = true;
}
void Transform::SetOffset(float x, float y, float z)
{
    SetOffset(XMFLOAT3(x, y, z));
}

void Transform::SetOffsetFromDirectionPoint(const XMFLOAT3& direction, const XMFLOAT3& pivot, float distance, bool directionIsLocal)
{
    SaveLastPosition();

    XMVECTOR dirV = XMLoadFloat3(&direction);
    dirV = XMVector3Normalize(dirV);

    if (directionIsLocal)
    {
        XMVECTOR quat = XMLoadFloat4(&m_quat);
        dirV = XMVector3Rotate(dirV, quat);
        dirV = XMVector3Normalize(dirV);
    }

    XMVECTOR pivotV = XMLoadFloat3(&pivot);
    XMVECTOR targetWorld = pivotV + dirV * distance;

    XMVECTOR posV = XMLoadFloat3(&m_pos);
    XMVECTOR offsetWorld = targetWorld - posV;

    XMVECTOR quat = XMLoadFloat4(&m_quat);
    XMVECTOR invQuat = XMQuaternionInverse(quat);
    XMVECTOR localOffset = XMVector3Rotate(offsetWorld, invQuat);

    XMFLOAT3 out;
    XMStoreFloat3(&out, localOffset);
    m_offset = out;

    m_dirty = true;
}

const XMFLOAT3& Transform::GetRotation() const
{
	return m_rot;
}

const XMMATRIX Transform::GetMatrixRotation() const
{
	return XMMatrixRotationX(m_rot.x) * XMMatrixRotationY(m_rot.y) * XMMatrixRotationZ(m_rot.z);
}

void Transform::SetRotation(const XMFLOAT3& r)
{
    m_rot = r;

    XMVECTOR q = XMQuaternionRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z);
    XMStoreFloat4(&m_quat, XMQuaternionNormalize(q));

    XMVECTOR qv = XMLoadFloat4(&m_quat);
    XMVECTOR rightV = XMVector3Rotate(XMVectorSet(1.f, 0.f, 0.f, 0.f), qv);
    XMVECTOR upV    = XMVector3Rotate(XMVectorSet(0.f, 1.f, 0.f, 0.f), qv);
    XMVECTOR lookV  = XMVector3Rotate(XMVectorSet(0.f, 0.f, 1.f, 0.f), qv);

    XMStoreFloat3(&mRight, rightV);
    XMStoreFloat3(&mUp, upV);
    XMStoreFloat3(&mLook, lookV);

    m_dirty = true;
}

void Transform::SetRotation(float x, float y, float z)
{
    SetRotation(XMFLOAT3(x, y, z));
}

const XMFLOAT4& Transform::LocalRotation() const
{
	return m_quat;
}

void Transform::SetLocalRotation(const XMFLOAT4& q)
{
    using namespace DirectX;
    // Normaliser l'entrée pour éviter la dérive
    XMVECTOR qv = XMLoadFloat4(&q);
    qv = XMQuaternionNormalize(qv);
    XMStoreFloat4(&m_quat, qv);

    // Mettre à jour les axes locaux
    XMVECTOR rightV = XMVector3Rotate(XMVectorSet(1.f, 0.f, 0.f, 0.f), qv);
    XMVECTOR upV    = XMVector3Rotate(XMVectorSet(0.f, 1.f, 0.f, 0.f), qv);
    XMVECTOR lookV  = XMVector3Rotate(XMVectorSet(0.f, 0.f, 1.f, 0.f), qv);

    XMStoreFloat3(&mRight, rightV);
    XMStoreFloat3(&mUp, upV);
    XMStoreFloat3(&mLook, lookV);

    m_dirty = true;
}

void Transform::AddLocalRotation(const XMFLOAT4& q)
{
    XMVECTOR qCurr = XMLoadFloat4(&m_quat);
    XMVECTOR qAdd = XMLoadFloat4(&q);
    qAdd = XMQuaternionNormalize(qAdd);

    XMVECTOR qRes = XMQuaternionNormalize(XMQuaternionMultiply(qCurr, qAdd));
    XMStoreFloat4(&m_quat, qRes);

    XMVECTOR rightV = XMVector3Rotate(XMVectorSet(1.f, 0.f, 0.f, 0.f), qRes);
    XMVECTOR upV    = XMVector3Rotate(XMVectorSet(0.f, 1.f, 0.f, 0.f), qRes);
    XMVECTOR lookV  = XMVector3Rotate(XMVectorSet(0.f, 0.f, 1.f, 0.f), qRes);

    XMStoreFloat3(&mRight, rightV);
    XMStoreFloat3(&mUp, upV);
    XMStoreFloat3(&mLook, lookV);

    m_dirty = true;
}

void Transform::AddLocalRotation(const XMFLOAT3& eulerDegrees)
{
    using namespace DirectX;

    XMVECTOR deltaQuat = XMQuaternionRotationRollPitchYaw(
        XMConvertToRadians(eulerDegrees.x),
        XMConvertToRadians(eulerDegrees.y),
        XMConvertToRadians(eulerDegrees.z)
    );
    deltaQuat = XMQuaternionNormalize(deltaQuat);

    XMVECTOR currentQuat = XMLoadFloat4(&m_quat);
    XMVECTOR result = XMQuaternionNormalize(XMQuaternionMultiply(deltaQuat, currentQuat));

    XMFLOAT4 out;
    XMStoreFloat4(&out, result);
    SetLocalRotation(out);
}

XMFLOAT4 Transform::GetLocalRotationQuaternion() const
{
    return m_quat;
}

const XMFLOAT3& Transform::GetScale() const
{
	return m_scale;
}

const XMMATRIX Transform::GetMatrixScale() const
{
	return XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
}

void Transform::SetScale(float x, float y, float z)
{
	m_scale = XMFLOAT3(x, y, z);
}

void Transform::LocalTranslate(Direction dir, float distance, float deltaTime )
{
    XMVECTOR forward = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&m_quat));
    XMVECTOR right = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), XMLoadFloat4(&m_quat));
    XMVECTOR up = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), XMLoadFloat4(&m_quat));
    XMVECTOR newdelta = XMVectorZero();

    switch (dir)
	{
	case Direction::Forward:
		newdelta = forward * distance;
		break;
	case Direction::Backward:
        newdelta = forward * -distance;
		break;
	case Direction::Right:
        newdelta = right * distance;
		break;
	case Direction::Left:
        newdelta = right * -distance;
		break;
	case Direction::Up:
        newdelta = up * distance;
		break;
	case Direction::Down:
        newdelta = up * -distance;
		break;
	}

    XMFLOAT3 deltaFloat3;
	XMStoreFloat3(&deltaFloat3, newdelta * deltaTime);
	Translate(deltaFloat3);
}

void Transform::GoForward(float distance, float deltaTime)
{
	LocalTranslate(Direction::Forward, distance, deltaTime);
}

void Transform::GoLeft(float distance, float deltaTime)
{
	LocalTranslate(Direction::Left, distance, deltaTime);
}

void Transform::GoRight(float distance, float deltaTime)
{
	LocalTranslate(Direction::Right, distance, deltaTime);
}

void Transform::GoBackward(float distance, float deltaTime)
{
	LocalTranslate(Direction::Backward, distance, deltaTime);
}

void Transform::GoUp(float distance, float deltaTime)
{
	LocalTranslate(Direction::Up, distance, deltaTime);
}

void Transform::GoDown(float distance, float deltaTime)
{
	LocalTranslate(Direction::Down, distance, deltaTime);
}

void Transform::Translate(const XMFLOAT3& delta)
{
    SaveLastPosition();
	m_pos = XMFLOAT3(m_pos.x + delta.x, m_pos.y + delta.y, m_pos.z + delta.z);
    m_dirty = true;
}

void Transform::Rotate(const XMFLOAT3& delta)
{
    m_rot = XMFLOAT3(m_rot.x + delta.x, m_rot.y + delta.y, m_rot.z + delta.z);

    XMVECTOR q = XMQuaternionRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z);
    XMStoreFloat4(&m_quat, XMQuaternionNormalize(q));
}

void Transform::GoToPostion(const XMFLOAT3& pos)
{
	Translate({ pos.x - m_pos.x, pos.y - m_pos.y, pos.z - m_pos.z });
}

void Transform::SlowDow(const XMFLOAT3& pos, float m)
{
    m /= 1000;

	XMFLOAT3 dif = { pos.x - m_pos.x, pos.y - m_pos.y, pos.z - m_pos.z };
	XMFLOAT3 dir = { static_cast<float>(dif.x * m), static_cast<float>(dif.y * m), static_cast<float>(dif.z * m) };

	Translate(dir);
}

void Transform::LookAt(const XMFLOAT3& target)
{
	XMVECTOR eyePos = XMLoadFloat3(&m_pos);
	XMVECTOR targetPos = XMLoadFloat3(&target);
	XMVECTOR upDir = XMVectorSet(0, 1, 0, 0);
	XMMATRIX lookAtMatrix = XMMatrixLookAtLH(eyePos, targetPos, upDir);
	XMMATRIX invLookAt = XMMatrixInverse(nullptr, lookAtMatrix);
	XMVECTOR rotQuat = XMQuaternionRotationMatrix(invLookAt);

	XMStoreFloat4(&m_quat, rotQuat);

	XMVECTOR axis;
	float angle;
	XMQuaternionToAxisAngle(&axis, &angle, rotQuat);
	XMFLOAT3 axisFloat3;
	XMStoreFloat3(&axisFloat3, axis);
	m_rot = XMFLOAT3(axisFloat3.x * angle, axisFloat3.y * angle, axisFloat3.z * angle);
}

void Transform::ResetAll()
{
    SaveLastPosition();

	m_pos = XMFLOAT3(0, 0, 0);
	m_rot = XMFLOAT3(0, 0, 0);
	m_scale = XMFLOAT3(1, 1, 1);
	m_quat = XMFLOAT4(0, 0, 0, 1);
	m_dirty = true;
}

void Transform::OrbitAroundAxis(const XMFLOAT3& center, const XMFLOAT3& axis, float radius, float angularSpeed, float dt)
{
    m_orbitAngle += angularSpeed * dt;

    XMVECTOR p = { m_pos.x * axis.x, m_pos.y * axis.y, m_pos.z * axis.z, };

    XMVECTOR centerVec = XMLoadFloat3(&center);
    XMVECTOR axisVec = XMLoadFloat3(&axis);
    XMVECTOR offset = XMVectorSet(radius, 0, 0, 0);
    XMMATRIX rotation = XMMatrixRotationAxis(axisVec, m_orbitAngle);
    XMVECTOR newPos = centerVec + XMVector3Transform(offset, rotation) + p;
        
    SaveLastPosition();

    XMStoreFloat3(&m_pos, newPos);
}

void Transform::LocalOscillation(const XMFLOAT3& axis, float amplitude, float frequency, float time)
{
    XMVECTOR axisVec = XMLoadFloat3(&axis);

    m_oscTime += time;
    float phase = frequency * m_oscTime;
    float newOsc = sinf(phase) * amplitude;

    if (!m_oscInitialized)
    {
        m_prevOsc = newOsc;
        m_oscInitialized = true;
    }

    float deltaOsc = newOsc - m_prevOsc;
    m_prevOsc = newOsc;

    XMVECTOR quat = XMLoadFloat4(&m_quat);
    XMVECTOR worldAxis = XMVector3Rotate(axisVec, quat);

    XMVECTOR deltaVec = worldAxis * deltaOsc;

    XMFLOAT3 deltaFloat3;
    XMStoreFloat3(&deltaFloat3, deltaVec);

    Translate(deltaFloat3);
}

void Transform::WorldOscillation(const XMFLOAT3& axis, float amplitude, float frequency, float time)
{
    XMVECTOR axisVec = XMLoadFloat3(&axis);

    m_oscTime += time;
    float phase = frequency * m_oscTime;
    float newOsc = sinf(phase) * amplitude;

    if (!m_oscInitialized)
    {
        m_prevOsc = newOsc;
        m_oscInitialized = true;
    }
	float deltaOsc = newOsc - m_prevOsc;

	m_prevOsc = newOsc;

	XMVECTOR deltaVec = axisVec * deltaOsc;
	XMFLOAT3 deltaFloat3;

	XMStoreFloat3(&deltaFloat3, deltaVec);

    Translate(deltaFloat3);
}

void Transform::SetPositionNoRecord(const XMFLOAT3& pos)
{
    m_pos = XMFLOAT3{ pos.x + m_offset.x, pos.y + m_offset.y, pos.z + m_offset.z };
    m_dirty = true;
}

void Transform::SetRotationDegrees(float xDeg, float yDeg, float zDeg)
{
    m_rot.x = XMConvertToRadians(xDeg);
    m_rot.y = XMConvertToRadians(yDeg);
    m_rot.z = XMConvertToRadians(zDeg);

    XMVECTOR q = XMQuaternionRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z);
    XMStoreFloat4(&m_quat, XMQuaternionNormalize(q));
    m_dirty = true;
}

void Transform::SetRotationDegrees(XMFLOAT3 rot)
{
	SetRotationDegrees(rot.x, rot.y, rot.z);
}

XMFLOAT4 Transform::GetQuat() const
{
	return m_quat;
}

void Transform::SetQuat(const XMFLOAT4& q)
{
    XMVECTOR qv = XMLoadFloat4(&q);
    qv = XMQuaternionNormalize(qv);
    XMStoreFloat4(&m_quat, qv);

    XMVECTOR rightV = XMVector3Rotate(XMVectorSet(1.f, 0.f, 0.f, 0.f), qv);
    XMVECTOR upV    = XMVector3Rotate(XMVectorSet(0.f, 1.f, 0.f, 0.f), qv);
    XMVECTOR lookV  = XMVector3Rotate(XMVectorSet(0.f, 0.f, 1.f, 0.f), qv);

    XMStoreFloat3(&mRight, rightV);
    XMStoreFloat3(&mUp, upV);
    XMStoreFloat3(&mLook, lookV);

    m_dirty = true;
}

void Transform::SaveLastPosition()
{
    m_lastPos = m_pos;
}