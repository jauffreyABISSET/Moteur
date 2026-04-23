#include "pch.h"
#include "MathHelper.h"

#include <cstdlib>
#include <cmath>
#include <cfloat>

using namespace DirectX;

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = 3.14159265358979323846f;

float MathHelper::RandF()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float MathHelper::RandF(float a, float b)
{
	return a + RandF() * (b - a);
}

int MathHelper::Rand(int a, int b)
{
	return a + rand() % ((b - a) + 1);
}

float MathHelper::AngleFromXY(float x, float y)
{
	float theta = atan2f(y, x); // returns value in [-PI, PI]
	if (theta < 0.0f)
		theta += 2.0f * Pi;
	return theta;
}

XMVECTOR MathHelper::SphericalToCartesian(float radius, float theta, float phi)
{
	return XMVectorSet(
		radius * sinf(phi) * cosf(theta),
		radius * cosf(phi),
		radius * sinf(phi) * sinf(theta),
		1.0f);
}

XMMATRIX MathHelper::InverseTranspose(CXMMATRIX M)
{
	XMMATRIX A = M;
	A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR det = XMMatrixDeterminant(A);
	return XMMatrixTranspose(XMMatrixInverse(&det, A));
}

XMFLOAT4X4 MathHelper::Identity4x4()
{
	static XMFLOAT4X4 I(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	return I;
}

float MathHelper::DistSq(const XMFLOAT3& a, const XMFLOAT3& b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	return dx * dx + dy * dy + dz * dz;
}

XMVECTOR MathHelper::RandUnitVec3()
{
	float z = RandF(-1.0f, 1.0f);
	float a = RandF(0.0f, 2.0f * Pi);
	float r = sqrtf(1.0f - z * z);

	return XMVectorSet(r * cosf(a), r * sinf(a), z, 0.0f);
}

XMVECTOR MathHelper::RandHemisphereUnitVec3(XMVECTOR n)
{
	XMVECTOR v = RandUnitVec3();
	if (XMVectorGetX(XMVector3Dot(v, n)) < 0.0f)
		v = XMVectorNegate(v);
	return v;
}

