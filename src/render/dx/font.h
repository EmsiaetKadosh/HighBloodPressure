//
// Created by EmsiaetKadosh on 25-4-12.
//

#pragma once

#include "dxdef.h"
#include "..\..\utils\IText.h"

class [[carlbeks::predecl, carlbeks::defineat("direct.h")]] DirectFontManager;
class [[carlbeks::predecl, carlbeks::defineat("direct.h")]] DirectX12Renderer;

class DirectFont final : public IFonts {
	friend class DirectFontManager;
	using Brush = ComPtr<ID2D1SolidColorBrush>;
	using TextFormat = ComPtr<IDWriteTextFormat>;
	using TextLayout = ComPtr<IDWriteTextLayout>;
	mutable TextFormat textFormat;
	DirectFontManager* manager;

	[[nodiscard]] const Brush& getBrush(unsigned int color) const;
	[[nodiscard]] int getWidth(const RenderableString::StringConfig& config) const noexcept override;
	[[nodiscard]] float drawSingle(const RenderableString::StringConfig& config, float x, float y, unsigned int defaultColor) const;
	void drawDirect(const RenderableString::StringConfig& config, float x, float y, unsigned int defaultColor) const;
	void tryCreate() const;
	DirectFont(DirectFontManager* const manager, const FontID id, const String& name, const double heightModifier, const double yOffset, const long escapement, const long orientation, const bool adaptAllSize): IFonts(id, name, heightModifier, yOffset, escapement, orientation, adaptAllSize), manager(manager) {}
	DirectFont(DirectFontManager* const manager, const FontID id, String&& name, const double heightModifier, const double yOffset, const long escapement, const long orientation, const bool adaptAllSize): IFonts(id, std::move(name), heightModifier, yOffset, escapement, orientation, adaptAllSize), manager(manager) {}

public:
	DirectFont(const DirectFont&) = default;
	DirectFont(DirectFont&&) = default;
	void draw(const RenderableString& text, int x, int y, unsigned color) const noexcept override;
	void drawCenter(const RenderableString& text, int x, int y, int w, int h, unsigned color) const noexcept override;
};

class DirectFontManager final : public IFontManager {
	friend class DirectFont;
	// D3D11On12 和 D2D/DW 相关
	ComPtr<ID3D11Device> device11;
	ComPtr<ID3D11DeviceContext> context11;
	ComPtr<ID3D11On12Device> deviceOn12;
	ComPtr<ID2D1Factory7> d2dFactory;
	ComPtr<IDWriteFactory7> dwFactory;
	ComPtr<ID2D1Device6> d2dDevice;
	ComPtr<ID2D1DeviceContext6> d2dContext;
	// 共享资源
	ComPtr<ID3D11Resource> textBufferShadow[DirectX12Configs::SwapFrameCount];
	ComPtr<ID2D1Bitmap1> d2dTargetBitmap[DirectX12Configs::SwapFrameCount];
	// 缓存
	mutable HashMap<unsigned int, ComPtr<ID2D1SolidColorBrush>> brushes;
	mutable HashMap<FontID, DirectFont> fonts;
	DirectFont* defaultFont = nullptr;
	FontID assigned = 0;
	bool isRenderingOnce = false;

	using IterBrush = HashMap<unsigned int, ComPtr<ID2D1SolidColorBrush>>::const_iterator;

	void prepare() noexcept(false);
	void terminate() noexcept(false);
	void renderStart();
	void renderEnd();
	[[nodiscard]] const ComPtr<ID2D1SolidColorBrush>& getBrush(unsigned int color) const;

public:
	DirectFontManager(DirectX12Renderer* renderer);

	void finalize() override;
	void resize(int width, int height) override;
	DirectFont& newFont(const String& name, double heightModifier, double yOffset, bool adaptAllSize, long escapement, long orientation) override;
	DirectFont& newFont(String&& name, double heightModifier, double yOffset, bool adaptAllSize, long escapement, long orientation) override;
	[[nodiscard]] DirectFont& getDefault() const noexcept override;
	[[nodiscard]] DirectFont& get(FontID id) const noexcept override;
};
