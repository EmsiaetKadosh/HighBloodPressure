//
// Created by EmsiaetKadosh on 2025/12/31.
//

#pragma once

#include <fstream>
#include <vector>

#include "directx/d3d12.h"
#include "directx/dxgiformat.h"
#include "src/render/dx/texture.hpp"

struct BMPLoadResult : DirectTextureCarrierExtraData {
	std::vector<uint8_t> pixelData; // RGBA数据
	unsigned int width;             // 纹理宽度
	unsigned int height;            // 纹理高度
	DXGI_FORMAT format;             // 纹理格式
	bool success;                   // 加载是否成功

	unsigned int getBufferSize() noexcept override { return pixelData.size(); }
	unsigned char* getBuffer() noexcept override { return pixelData.data(); }
};

template<typename T>
class ResourceGuard {
	T* resource = nullptr;

public:
	ResourceGuard(T* resource) noexcept : resource(resource) {}
	~ResourceGuard() noexcept { if (resource) delete resource; }
	T* get() const noexcept { return resource; }
	void detach() noexcept { resource = nullptr; }

	void join(T* const r) noexcept {
		if (resource) delete resource;
		resource = r;
	}
};

class TextureLoaderBMP : public DirectTextureLoader {
protected:
	bool onLoad(DirectTextureCarrier& carrier) noexcept override {
		ResourceGuard<BMPLoadResult> pr = nullptr;
		try { pr.join(new BMPLoadResult); }
		catch (std::bad_alloc&) {
			renderer.getLogger().ofNoexcept("Failed to load BMP: BadAlloc");
			return false;
		}
		BMPLoadResult& result = *pr.get();

		// 1. 读取文件
		std::ifstream file(carrier.getFile(), std::ios::binary);
		if (!file.is_open()) return false;
		// 2. 读取文件头
		BITMAPFILEHEADER fileHeader;
		file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
		// 验证BMP格式
		if (fileHeader.bfType != 0x4D42) return false; // "BM"
		// 3. 读取信息头
		BITMAPINFOHEADER infoHeader;
		file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
		// 只支持常见的未压缩格式
		if (infoHeader.biCompression != 0) return false; // BI_RGB
		// 只支持24bpp或32bpp
		if (infoHeader.biBitCount != 24 && infoHeader.biBitCount != 32) return false;
		result.width = infoHeader.biWidth;
		result.height = abs(infoHeader.biHeight); // 处理倒序存储
		const bool isTopDown = infoHeader.biHeight < 0;
		// 4. 计算像素数据参数
		const unsigned long long bytesPerPixel = infoHeader.biBitCount / 8;
		const unsigned long long rowPitch = result.width * bytesPerPixel + 3 & ~3; // 4字节对齐
		const unsigned long long pixelDataSize = rowPitch * result.height;
		// 5. 定位并读取像素数据
		file.seekg(fileHeader.bfOffBits, std::ios::beg);
		std::vector<uint8_t> bmpPixelData(pixelDataSize);
		file.read(reinterpret_cast<char*>(bmpPixelData.data()), pixelDataSize);
		// 6. 转换为DXGI格式 (BGRA -> RGBA)
		result.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		result.pixelData.resize(result.width * result.height * 4); // RGBA
		for (uint32_t y = 0; y < result.height; ++y) {
			// BMP可能是倒序存储，需要翻转
			uint32_t srcY = isTopDown ? y : (result.height - 1 - y);
			const uint8_t* srcRow = bmpPixelData.data() + srcY * rowPitch;
			uint8_t* dstRow = result.pixelData.data() + y * result.width * 4;

			for (uint32_t x = 0; x < result.width; ++x) {
				const uint8_t* srcPixel = srcRow + x * bytesPerPixel;
				uint8_t* dstPixel = dstRow + x * 4;

				if (bytesPerPixel == 3) {    // 24bpp BGR -> RGBA
					dstPixel[0] = srcPixel[2]; // R
					dstPixel[1] = srcPixel[1]; // G
					dstPixel[2] = srcPixel[0]; // B
					dstPixel[3] = 255;         // A
				}
				else {                       // 32bpp BGRA -> RGBA
					dstPixel[0] = srcPixel[2]; // R
					dstPixel[1] = srcPixel[1]; // G
					dstPixel[2] = srcPixel[0]; // B
					dstPixel[3] = srcPixel[3]; // A
				}
			}
		}
		pr.detach();
		return true;
	}

	void onUnload(DirectTextureCarrier& carrier) noexcept override {}
	bool onPrepare(DirectTextureCarrier& carrier) noexcept override { return true; }
	void onRelease(DirectTextureCarrier& carrier) noexcept override {}

public:
	using DirectTextureLoader::DirectTextureLoader;
};

BMPLoadResult LoadBMPToD3D12Buffer(const std::wstring& filename) {
	BMPLoadResult result = {};
	// 1. 读取文件
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		result.success = false;
		return result;
	}
	// 2. 读取文件头
	BITMAPFILEHEADER fileHeader;
	file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
	// 验证BMP格式
	if (fileHeader.bfType != 0x4D42) { // "BM"
		result.success = false;
		return result;
	}
	// 3. 读取信息头
	BITMAPINFOHEADER infoHeader;
	file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
	// 只支持常见的未压缩格式
	if (infoHeader.biCompression != 0) { // BI_RGB
		result.success = false;
		return result;
	}
	// 只支持24bpp或32bpp
	if (infoHeader.biBitCount != 24 && infoHeader.biBitCount != 32) {
		result.success = false;
		return result;
	}
	result.width = infoHeader.biWidth;
	result.height = abs(infoHeader.biHeight); // 处理倒序存储
	const bool isTopDown = infoHeader.biHeight < 0;
	// 4. 计算像素数据参数
	const unsigned long long bytesPerPixel = infoHeader.biBitCount / 8;
	const unsigned long long rowPitch = result.width * bytesPerPixel + 3 & ~3; // 4字节对齐
	const unsigned long long pixelDataSize = rowPitch * result.height;
	// 5. 定位并读取像素数据
	file.seekg(fileHeader.bfOffBits, std::ios::beg);
	std::vector<uint8_t> bmpPixelData(pixelDataSize);
	file.read(reinterpret_cast<char*>(bmpPixelData.data()), pixelDataSize);
	// 6. 转换为DXGI格式 (BGRA -> RGBA)
	result.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	result.pixelData.resize(result.width * result.height * 4); // RGBA
	for (uint32_t y = 0; y < result.height; ++y) {
		// BMP可能是倒序存储，需要翻转
		uint32_t srcY = isTopDown ? y : (result.height - 1 - y);
		const uint8_t* srcRow = bmpPixelData.data() + srcY * rowPitch;
		uint8_t* dstRow = result.pixelData.data() + y * result.width * 4;

		for (uint32_t x = 0; x < result.width; ++x) {
			const uint8_t* srcPixel = srcRow + x * bytesPerPixel;
			uint8_t* dstPixel = dstRow + x * 4;

			if (bytesPerPixel == 3) {    // 24bpp BGR -> RGBA
				dstPixel[0] = srcPixel[2]; // R
				dstPixel[1] = srcPixel[1]; // G
				dstPixel[2] = srcPixel[0]; // B
				dstPixel[3] = 255;         // A
			}
			else {                       // 32bpp BGRA -> RGBA
				dstPixel[0] = srcPixel[2]; // R
				dstPixel[1] = srcPixel[1]; // G
				dstPixel[2] = srcPixel[0]; // B
				dstPixel[3] = srcPixel[3]; // A
			}
		}
	}

	result.success = true;
	return result;
}

