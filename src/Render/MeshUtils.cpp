#include "pch.h"
#include "MeshUtils.h"

void MeshUtils::ComputeVertexNormals(std::vector<Vertex>& verts, const std::vector<uint32_t>& indices)
{
    // zero normals
    for (auto &v : verts) v.Normal = {0.0f, 0.0f, 0.0f};

    // accumulate face normals
    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        uint32_t i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];

        XMVECTOR p0 = XMLoadFloat3(&verts[i0].position);
        XMVECTOR p1 = XMLoadFloat3(&verts[i1].position);
        XMVECTOR p2 = XMLoadFloat3(&verts[i2].position);

        XMVECTOR e1 = XMVectorSubtract(p1, p0);
        XMVECTOR e2 = XMVectorSubtract(p2, p0);
        XMVECTOR n = XMVector3Cross(e1, e2);
        n = XMVector3Normalize(n);

        XMFLOAT3 fn;
        XMStoreFloat3(&fn, n);

        verts[i0].Normal.x += fn.x; verts[i0].Normal.y += fn.y; verts[i0].Normal.z += fn.z;
        verts[i1].Normal.x += fn.x; verts[i1].Normal.y += fn.y; verts[i1].Normal.z += fn.z;
        verts[i2].Normal.x += fn.x; verts[i2].Normal.y += fn.y; verts[i2].Normal.z += fn.z;
    }

    // normalize per-vertex
    for (auto &v : verts)
    {
        using namespace DirectX;
        XMVECTOR n = XMLoadFloat3(&v.Normal);
        n = XMVector3Normalize(n);
        XMFLOAT3 nn; XMStoreFloat3(&nn, n);
        v.Normal = nn;
    }
}
