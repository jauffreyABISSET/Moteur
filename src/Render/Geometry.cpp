#include "pch.h"
#include "Geometry.h"
#include <sstream>

const void Geometry::Bind(ID3D12GraphicsCommandList* cmd)
{
    if (!m_vertexBuffer || !m_indexBuffer || m_indexCount == 0)
    {
        OutputDebugStringA("DEBUG: Geometry::Bind - invalid geometry, skipping IA setup\n");
        return;
    }

    cmd->IASetVertexBuffers(0, 1, &m_VBV);
    cmd->IASetIndexBuffer(&m_IBV);
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

const uint32_t Geometry::GetIndexCount()
{
    return m_indexCount;
}