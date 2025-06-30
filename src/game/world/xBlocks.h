//
// Created by EmsiaetKadosh on 25-5-8.
//

#pragma once

#include "Block.h"

class BrickBlock final : public Block {
	friend class Game;
	TextureEntry textureEntry = textureManager.getTexture(L"hbp\\block\\brick");
	BrickBlock(const BlockLocation& location) : Block(location) {}

public:
	void tick() noexcept(false) override {}
	void render(double tickDelta, QWORD tickRendering) const noexcept override { renderer.textureWorld(textureEntry, getLocation().getPosition()); }
	static BrickBlock* create(const BlockLocation& location) noexcept { return allocatedFor(new BrickBlock(location)); }
	[[nodiscard]] Description getDescription() const override { return Description().title(L"Brick"_renderable); }
};
