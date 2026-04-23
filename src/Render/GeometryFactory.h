#pragma once
#include <d3d12.h>
#include <vector>
#include "Struct.h"
#include "Material.h"

enum class PrimitiveType
{
	Cube,
	Sphere,
	Plane,
	Cylinder,
	Pyramid
};

class Geometry;

class GeometryFactory
{
	static std::string m_name;
	PrimitiveType m_primitiveType;

	static std::unordered_map<std::string, Geometry*> m_geometries;

public:
    void Initialize();

	static Geometry* CreateCube(MaterialConstants constanMat = { {1.f, 1.f, 1.f, 1.f}, 0.5f, 0.5f });
    static Geometry* CreateSphere(MaterialConstants constanMat = { {1.f, 1.f, 1.f, 1.f}, 0.5f, 0.5f });
	static Geometry* CreatePlane(MaterialConstants constanMat = { {1.f, 1.f, 1.f, 1.f}, 0.5f, 0.5f });
	static Geometry* CreateCylinder(MaterialConstants constanMat = { {1.f, 1.f, 1.f, 1.f}, 0.5f, 0.5f });
    static Geometry* CreatePyramid(MaterialConstants constanMat = { {1.f, 1.f, 1.f, 1.f}, 0.5f, 0.5f });
	static Geometry* CreateCustom(Vertex* vertices, size_t vertexCount, uint32_t* indices, size_t indexCount, bool calculateNormals);

	static Geometry* GetGeometry(PrimitiveType type, MaterialConstants constanMat = { {1.f, 1.f, 1.f, 1.f}, 0.5f, 0.5f });

	static std::string GetName() { return m_name; }
	static Geometry* GetGeometryByName(const std::string& name);

	static Geometry* Bind(Geometry* geo, Vertex* vertices, size_t vertexCount, uint32_t* indices, size_t indexCount);

};