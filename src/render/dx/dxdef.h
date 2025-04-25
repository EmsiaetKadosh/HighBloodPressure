//
// Created by EmsiaetKadosh on 25-4-12.
//

#pragma once

// DirectX
// #include <d3d12.h> // 估计是版本过于老旧了，用不了一点
#include "..\..\include\include\directx\d3d12.h"
#include "..\..\include\include\directx\d3dx12.h"
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#ifdef __CARLBEKS_DEBUG__
#include <dxgidebug.h>
#endif
#include <wrl.h>
// DirectX

#include "..\..\def.h"
#include "..\..\hbp.h"
#include "..\..\utils\exception.h"
#include "..\..\utils\Chars.h"
#include "..\..\utils\IText.h"
#include "..\Renderer.h"

#include <d3d11on12.h>
#include <d2d1_3.h>
#include <dwrite_3.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "D3DCompiler.lib")

#define discard_return(exp) (exp)

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

struct DirectX12Configs {
	static constexpr QWORD SwapFrameCount = 2;
};
