#pragma once

//DirectX12
#include <windows.h>
#include <WindowsX.h>
#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

//Tools
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <list>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include <winstring.h>
#include <iostream>
#include <filesystem>


//Core
#include "Core/d3dx12.h"
#include <Core/Utils.h>
#include "Core/DebugConsole.h"

//Engine
#include "Engine/AppManager.h"
#include "Engine/InputsMethods.h"
#include "Engine/GameTimer.h"
#include "Engine/GameManager.h"
#include "Engine/Entity.h"
#include "Engine/Prefab.h"
#include "Engine/Script.h"
#include "Engine/Component.h"
#include "Engine/ComponentMeshRenderer.h"
#include "Engine/ComponentGravity.h"
#include "Engine/ComponentCollider.h"
#include "Engine/ComponentBoxCollider.h"
#include "Engine/ComponentSphereCollider.h"
#include "Engine/ComponentRigidBody.h"
#include "Engine/ComponentText.h"
#include "Engine/ComponentLight.h"
#include "Engine/SceneManager.h"
#include "Engine/Scene.h"
#include "Engine/StateMachine.h"
#include "Engine/JsonExporter.h"

// Render
#include "Render/Window.h"
#include "Render/Camera.h"
#include "Render/RenderSystem.h"
#include "Render/GeometryFactory.h"
#include "Render/Material.h"
#include "Render/Texture.h"
#include "Render/Geometry.h"
#include "Render/Color.h"
#include "Render/Struct.h"
#include "Render/LightManager.h"
#include "Render/Font.h"
#include "Render/FontManager.h"
#include "Render/DescriptorHeap.h"
