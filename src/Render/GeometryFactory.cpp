#include "pch.h"
#include "GeometryFactory.h"

std::string GeometryFactory::m_name = "GeometryFactory";
std::unordered_map<std::string, Geometry*> GeometryFactory::m_geometries;

namespace {

    static void ComputeNormals(Vertex* vertices, size_t vertexCount, const uint32_t* indices, size_t indexCount)
    {
        if (!vertices || !indices) return;

        XMVECTOR center = XMVectorZero();
        for (size_t i = 0; i < vertexCount; ++i)
        {
            center = XMVectorAdd(center, XMLoadFloat3(&vertices[i].position));
        }
        center = XMVectorScale(center, 1.0f / static_cast<float>(vertexCount));
        XMFLOAT3 modelCenter; XMStoreFloat3(&modelCenter, center);

        for (size_t i = 0; i < vertexCount; ++i)
            vertices[i].Normal = { 0.0f, 0.0f, 0.0f };

        for (size_t i = 0; i + 2 < indexCount; i += 3)
        {
            uint32_t i0 = indices[i + 0];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            if (i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount)
                continue;

            XMVECTOR p0 = XMLoadFloat3(&vertices[i0].position);
            XMVECTOR p1 = XMLoadFloat3(&vertices[i1].position);
            XMVECTOR p2 = XMLoadFloat3(&vertices[i2].position);

            XMVECTOR e1 = XMVectorSubtract(p1, p0);
            XMVECTOR e2 = XMVectorSubtract(p2, p0);
            XMVECTOR fn = XMVector3Cross(e1, e2);

            XMVECTOR fnLen = XMVector3Length(fn);
            float len = XMVectorGetX(fnLen);
            if (len <= 1e-6f) continue;

            XMVECTOR centroid = XMVectorScale(XMVectorAdd(XMVectorAdd(p0, p1), p2), 1.0f / 3.0f);
            XMFLOAT3 fc; XMStoreFloat3(&fc, centroid);

            XMFLOAT3 mc = modelCenter;
            XMFLOAT3 fnf;
            XMStoreFloat3(&fnf, fn);

            XMVECTOR vecToCenter = XMVectorSubtract(centroid, center);
            XMVECTOR vecToCenterLen = XMVector3Length(vecToCenter);
            float vlen = XMVectorGetX(vecToCenterLen);

            if (vlen > 1e-6f)
            {
                float dot = fnf.x * (fc.x - mc.x) + fnf.y * (fc.y - mc.y) + fnf.z * (fc.z - mc.z);
                if (dot < 0.0f)
                {
                    fn = XMVectorNegate(fn);
                    XMStoreFloat3(&fnf, fn);
                }
            }
            else
            {
                float upDot = fnf.y;
                if (upDot < 0.0f)
                {
                    fn = XMVectorNegate(fn);
                    XMStoreFloat3(&fnf, fn);
                }
            }

            fn = XMVector3Normalize(fn);
            XMFLOAT3 f; XMStoreFloat3(&f, fn);

            vertices[i0].Normal.x += f.x; vertices[i0].Normal.y += f.y; vertices[i0].Normal.z += f.z;
            vertices[i1].Normal.x += f.x; vertices[i1].Normal.y += f.y; vertices[i1].Normal.z += f.z;
            vertices[i2].Normal.x += f.x; vertices[i2].Normal.y += f.y; vertices[i2].Normal.z += f.z;
        }

        for (size_t i = 0; i < vertexCount; ++i)
        {
            XMVECTOR n = XMLoadFloat3(&vertices[i].Normal);
            XMVECTOR nlen = XMVector3Length(n);
            float ln = XMVectorGetX(nlen);
            if (ln > 1e-6f)
            {
                n = XMVector3Normalize(n);
                XMFLOAT3 nn; XMStoreFloat3(&nn, n);
                vertices[i].Normal = nn;
            }
            else
            {
                vertices[i].Normal = { 0.0f, 1.0f, 0.0f };
            }
        }
    }
}

Geometry* GeometryFactory::GetGeometryByName(const std::string& name)
{
    if (name == "Cube") return GeometryFactory::CreateCube();
    if (name == "Sphere") return GeometryFactory::CreateSphere();
    if (name == "Plane") return GeometryFactory::CreatePlane();
    if (name == "Cylinder") return GeometryFactory::CreateCylinder();
    return nullptr;
}

Geometry* GeometryFactory::CreateCube(MaterialConstants constanMat)
{
    Geometry* geo = new Geometry();

    float size = 1.0f;

    m_name = "Cube";

    Vertex vertices[] =
    {
        // position                     color                                   normal           uv
        {{-size,-size,-size}, Color::NormalizeColor(constanMat.color), {0,0,0}, {0,0}},
        {{ size,-size,-size}, Color::NormalizeColor(constanMat.color), {0,0,0}, {1,0}},
        {{ size, size,-size}, Color::NormalizeColor(constanMat.color), {0,0,0}, {1,1}},
        {{-size, size,-size}, Color::NormalizeColor(constanMat.color), {0,0,0}, {0,1}},
        {{-size,-size, size}, Color::NormalizeColor(constanMat.color), {0,0,0}, {0,0}},
        {{ size,-size, size}, Color::NormalizeColor(constanMat.color), {0,0,0}, {1,0}},
        {{ size, size, size}, Color::NormalizeColor(constanMat.color), {0,0,0}, {1,1}},
        {{-size, size, size}, Color::NormalizeColor(constanMat.color), {0,0,0}, {0,1}}
    };

    uint32_t indices[] =
    {
        0,1,2, 0,2,3,
        4,6,5, 4,7,6,
        4,5,1, 4,1,0,
        3,2,6, 3,6,7,
        1,5,6, 1,6,2,
        4,0,3, 4,3,7
    };

    geo->m_indexCount = 36;

    ComputeNormals(vertices, _countof(vertices), indices, _countof(indices));

    Bind(geo, vertices, _countof(vertices), indices, _countof(indices));

	m_geometries.emplace(m_name, geo);

    return geo;
}

Geometry* GeometryFactory::CreatePyramid(MaterialConstants constanMat)
{
    Geometry* geo = new Geometry();

	m_name = "Pyramid";

    float size = 1.0f;
    float height = 1.5f;

    Vertex vertices[] =
    {
        // Base
        {{-size, 0, -size}, Color::NormalizeColor(constanMat.color), {0,0,0}, {0,0}},
        {{ size, 0, -size}, Color::NormalizeColor(constanMat.color), {0,0,0}, {1,0}},
        {{ size, 0,  size}, Color::NormalizeColor(constanMat.color), {0,0,0}, {1,1}},
        {{-size, 0,  size}, Color::NormalizeColor(constanMat.color), {0,0,0}, {0,1}},

        // Top
        {{0, height, 0},    Color::NormalizeColor(constanMat.color), {0,0,0}, {0.5f,0.5f}}
    };

    uint32_t indices[] =
    {
        // Base
        0,1,2,
        0,2,3,

        // Faces
        0,1,4,
        1,2,4,
        2,3,4,
        3,0,4
    };

    geo->m_indexCount = 18;

    ComputeNormals(vertices, _countof(vertices), indices, _countof(indices));

    Bind(geo, vertices, _countof(vertices), indices, _countof(indices));

    m_geometries.emplace(m_name, geo);

    return geo;
}

Geometry* GeometryFactory::CreateSphere(MaterialConstants constantMat)
{
    Geometry* geo = new Geometry();

    m_name = "Sphere";

    const unsigned int latitudeSegments = 16;
    const unsigned int longitudeSegments = 32;
    const float radius = 0.5f;
    const float PI = 3.14159265358979323846f;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve((latitudeSegments + 1) * (longitudeSegments + 1));
    indices.reserve(latitudeSegments * longitudeSegments * 6);

    for (unsigned int lat = 0; lat <= latitudeSegments; ++lat)
    {
        float v = static_cast<float>(lat) / static_cast<float>(latitudeSegments);
        float theta = v * PI;
        float sinTheta = sinf(theta);
        float cosTheta = cosf(theta);

        for (unsigned int lon = 0; lon <= longitudeSegments; ++lon)
        {
            float u = static_cast<float>(lon) / static_cast<float>(longitudeSegments);
            float phi = u * 2.0f * PI;
            float sinPhi = sinf(phi);
            float cosPhi = cosf(phi);

            float x = cosPhi * sinTheta * radius;
            float y = cosTheta * radius;
            float z = sinPhi * sinTheta * radius;

            vertices.push_back({ { x, y, z }, Color::NormalizeColor(constantMat.color), {0,0,0}, { u, 1.0f - v } });
        }
    }

    for (unsigned int lat = 0; lat < latitudeSegments; ++lat)
    {
        for (unsigned int lon = 0; lon < longitudeSegments; ++lon)
        {
            uint32_t first = lat * (longitudeSegments + 1) + lon;
            uint32_t second = first + (longitudeSegments + 1);

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    ComputeNormals(vertices.data(), vertices.size(), indices.data(), indices.size());

    Bind(geo, vertices.data(), vertices.size(), indices.data(), indices.size());

    m_geometries.emplace(m_name, geo);

    return geo;
}

Geometry* GeometryFactory::CreatePlane(MaterialConstants constantMat)
{
    Geometry* geo = new Geometry();
    float size = 1.0f;

	m_name = "Plane";

    Vertex vertices[] =
    {
        {{-size,-0.1f,-size}, Color::NormalizeColor(constantMat.color), {0,0,0}, {0,0}},
        {{ size,-0.1f,-size}, Color::NormalizeColor(constantMat.color), {0,0,0}, {1,0}},
        {{ size,-0.1f, size}, Color::NormalizeColor(constantMat.color), {0,0,0}, {1,1}},
        {{-size,-0.1f, size}, Color::NormalizeColor(constantMat.color), {0,0,0}, {0,1}},
        {{-size,0.1f,-size}, Color::NormalizeColor(constantMat.color), {0,0,0}, {0,0} },
        {{ size,0.1f,-size}, Color::NormalizeColor(constantMat.color), {0,0,0}, {1,0}},
        {{ size,0.1f, size}, Color::NormalizeColor(constantMat.color), {0,0,0}, {1,1}},
        {{-size,0.1f, size}, Color::NormalizeColor(constantMat.color), {0,0,0}, {0,1}}
    };

    uint32_t indices[] =
    {
        0,2,1, 0,3,2,
		4,5,6, 4,6,7
    };

    geo->m_indexCount = 12;

    ComputeNormals(vertices, _countof(vertices), indices, _countof(indices));

    Bind(geo, vertices, _countof(vertices), indices, _countof(indices));

    m_geometries.emplace(m_name, geo);

    return geo;
}

Geometry* GeometryFactory::CreateCylinder(MaterialConstants constanMat)
{
    Geometry* geo = new Geometry();

	m_name = "Cylinder";

    const unsigned int slices = 16;
    const float radius = 0.5f;
    const float height = 1.0f;
    const float halfH = height * 0.5f;
    const float PI = 3.14159265358979323846f;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(slices * 2 + 2);
    indices.reserve(slices * 12);

    for (unsigned int i = 0; i < slices; ++i)
    {
        float u = static_cast<float>(i) / static_cast<float>(slices);
        float theta = u * 2.0f * PI;
        float cx = cosf(theta) * radius;
        float cz = sinf(theta) * radius;

        vertices.push_back({ { cx,  halfH, cz }, Color::NormalizeColor(constanMat.color), {0,0,0}, { u, 0.0f } }); // top
        vertices.push_back({ { cx, -halfH, cz }, Color::NormalizeColor(constanMat.color), {0,0,0}, { u, 1.0f } }); // bottom
    }

    for (unsigned int i = 0; i < slices; ++i)
    {
        uint32_t top0 = i * 2;
        uint32_t bot0 = top0 + 1;
        uint32_t top1 = ((i + 1) % slices) * 2;
        uint32_t bot1 = top1 + 1;

        indices.push_back(top0);
        indices.push_back(bot0);
        indices.push_back(top1);

        indices.push_back(top1);
        indices.push_back(bot0);
        indices.push_back(bot1);
    }

    uint32_t topCenter = static_cast<uint32_t>(vertices.size());
    vertices.push_back({ { 0.0f, halfH, 0.0f }, Color::NormalizeColor(constanMat.color), {0,0,0}, { 0.5f, 0.5f } });
    for (unsigned int i = 0; i < slices; ++i)
    {
        uint32_t a = topCenter;
        uint32_t b = ( (i + 1) % slices ) * 2;
        uint32_t c = i * 2;
        indices.push_back(a);
        indices.push_back(b);
        indices.push_back(c);
    }

    uint32_t botCenter = static_cast<uint32_t>(vertices.size());
    vertices.push_back({ { 0.0f, -halfH, 0.0f }, Color::NormalizeColor(constanMat.color), {0,0,0}, { 0.5f, 0.5f } });
    for (unsigned int i = 0; i < slices; ++i)
    {
        uint32_t a = botCenter;
        uint32_t b = i * 2 + 1;
        uint32_t c = ((i + 1) % slices) * 2 + 1;
        indices.push_back(a);
        indices.push_back(b);
        indices.push_back(c);
    }

    ComputeNormals(vertices.data(), vertices.size(), indices.data(), indices.size());

    Bind(geo, vertices.data(), vertices.size(), indices.data(), indices.size());

    m_geometries.emplace(m_name, geo);

    return geo;
}

Geometry* GeometryFactory::GetGeometry(PrimitiveType type, MaterialConstants constanMat)
{
    switch (type)
    {
    case PrimitiveType::Cube:
        if (m_geometries.find("Cube") != m_geometries.end())
            return m_geometries["Cube"];
        return CreateCube(constanMat);
    case PrimitiveType::Pyramid:
        if (m_geometries.find("Pyramid") != m_geometries.end())
            return m_geometries["Pyramid"];
        return CreatePyramid(constanMat);
    case PrimitiveType::Sphere:
        if (m_geometries.find("Sphere") != m_geometries.end())
            return m_geometries["Sphere"];
        return CreateSphere(constanMat);
    case PrimitiveType::Plane:
        if (m_geometries.find("Plane") != m_geometries.end())
            return m_geometries["Plane"];
        return CreatePlane(constanMat);
    case PrimitiveType::Cylinder:
        if (m_geometries.find("Cylinder") != m_geometries.end())
            return m_geometries["Cylinder"];
        return CreateCylinder(constanMat);
    default:
        OutputDebugStringA("GeometryFactory::GetGeometry: unknown primitive type\n");
        break;
    }

    return nullptr;
}

Geometry* GeometryFactory::Bind(Geometry* geo, Vertex* vertices, size_t vertexCount, uint32_t* indices, size_t indexCount)
{
    ID3D12Device* device = RenderSystem::Get().GetDevice();
    ID3D12CommandQueue* queue = RenderSystem::Get().GetCommandQueue();

    if (device == nullptr) {
        OutputDebugStringA("GeometryFactory::Bind failed: device is null\n");
        return nullptr;
    }
    if (queue == nullptr) {
        OutputDebugStringA("GeometryFactory::Bind failed: command queue is null\n");
        return nullptr;
    }

    // --- 1. Créer command allocator + command list temporaire ---
    ID3D12CommandAllocator* allocator = nullptr;
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));

    ID3D12GraphicsCommandList* tempCmd = nullptr;
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&tempCmd)));

    // --- 2. Upload Vertex Buffer ---
    {
        const UINT vbSize = static_cast<UINT>(sizeof(Vertex) * vertexCount);
        CD3DX12_HEAP_PROPERTIES heapDefault(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(vbSize);

        // Create default buffer in COMMON (avoid CreateCommittedResource initial state ignored warning)
        ThrowIfFailed(device->CreateCommittedResource(&heapDefault, D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&geo->m_vertexBuffer)));

        // Transition COMMON -> COPY_DEST before copying
        {
            CD3DX12_RESOURCE_BARRIER barrierToCopy = CD3DX12_RESOURCE_BARRIER::Transition(
                geo->m_vertexBuffer,
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_COPY_DEST);
            tempCmd->ResourceBarrier(1, &barrierToCopy);
        }

        // Upload buffer kept in geo->m_vertexUploader
        ID3D12Resource* vbUpload;
        CD3DX12_HEAP_PROPERTIES heapUpload(D3D12_HEAP_TYPE_UPLOAD);
        ThrowIfFailed(device->CreateCommittedResource(&heapUpload, D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vbUpload)));

        void* data = nullptr;
        ThrowIfFailed(vbUpload->Map(0, nullptr, &data));
        memcpy(data, vertices, vbSize);
        vbUpload->Unmap(0, nullptr);

        tempCmd->CopyBufferRegion(geo->m_vertexBuffer, 0, vbUpload, 0, vbSize);

        // Transition COPY_DEST -> VERTEX_AND_CONSTANT_BUFFER
        CD3DX12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            geo->m_vertexBuffer,
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        tempCmd->ResourceBarrier(1, &resBarrier);

        geo->m_VBV.BufferLocation = geo->m_vertexBuffer->GetGPUVirtualAddress();
        geo->m_VBV.SizeInBytes = vbSize;
        geo->m_VBV.StrideInBytes = sizeof(Vertex);

        // keep upload buffer alive until GPU finished: store in geo
        geo->m_vertexUploader = vbUpload;
    }

    // --- 3. Upload Index Buffer ---
    {
        const UINT ibSize = static_cast<UINT>(sizeof(uint32_t) * indexCount);
        CD3DX12_HEAP_PROPERTIES heapDefault(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(ibSize);

        // Create default index buffer in COMMON
        ThrowIfFailed(device->CreateCommittedResource(&heapDefault, D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&geo->m_indexBuffer)));

        // Transition COMMON -> COPY_DEST before copying
        {
            CD3DX12_RESOURCE_BARRIER barrierToCopy = CD3DX12_RESOURCE_BARRIER::Transition(
                geo->m_indexBuffer,
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_COPY_DEST);
            tempCmd->ResourceBarrier(1, &barrierToCopy);
        }

        // Upload buffer kept in geo->m_indexUploader
        ID3D12Resource* ibUpload;
        CD3DX12_HEAP_PROPERTIES heapUpload(D3D12_HEAP_TYPE_UPLOAD);

        ThrowIfFailed(device->CreateCommittedResource(&heapUpload, D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&ibUpload)));

        void* data = nullptr;
        ThrowIfFailed(ibUpload->Map(0, nullptr, &data));
        memcpy(data, indices, ibSize);
        ibUpload->Unmap(0, nullptr);

        tempCmd->CopyBufferRegion(geo->m_indexBuffer, 0, ibUpload, 0, ibSize);

        CD3DX12_RESOURCE_BARRIER resBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
            geo->m_indexBuffer,
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_INDEX_BUFFER);

        tempCmd->ResourceBarrier(1, &resBarrier2);

        geo->m_IBV.BufferLocation = geo->m_indexBuffer->GetGPUVirtualAddress();
        geo->m_IBV.Format = DXGI_FORMAT_R32_UINT;
        geo->m_IBV.SizeInBytes = ibSize;

        // keep upload buffer alive until GPU finished: store in geo
        geo->m_indexUploader = ibUpload;
    }

    // --- 4. Close + execute command list + wait fence ---
    ThrowIfFailed(tempCmd->Close());
    ID3D12CommandList* lists[] = { tempCmd };
    queue->ExecuteCommandLists(1, lists);

    // Fence
    ID3D12Fence* fence = nullptr;
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    UINT64 fenceValue = 1;
    HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    ThrowIfFailed(queue->Signal(fence, fenceValue));

    if (fence->GetCompletedValue() < fenceValue)
    {
        ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, eventHandle));
		if (eventHandle != 0)
            WaitForSingleObject(eventHandle, INFINITE);
    }
    if (eventHandle) CloseHandle(eventHandle);

    geo->DisposeUploaders();

    geo->m_indexCount = static_cast<uint32_t>(indexCount);

    // --- LIBÉRATION DES RESSOURCES LOCALES pour éviter fuites ---
    if (tempCmd) { tempCmd->Release(); tempCmd = nullptr; }
    if (allocator) { allocator->Release(); allocator = nullptr; }
    if (fence) { fence->Release(); fence = nullptr; }

    return geo;
}

Geometry* GeometryFactory::CreateCustom(Vertex* vertices, size_t vertexCount, uint32_t* indices, size_t indexCount, bool calculateNormals)
{
    Geometry* geo = new Geometry();

    if (calculateNormals)
        ComputeNormals(vertices, vertexCount, indices, indexCount);

    Bind(geo, vertices, vertexCount, indices, indexCount);

    return geo;
}