//
// Created by EmsiaetKadosh on 25-3-21.
//

#pragma once

#include "..\..\def.h"
#include "..\..\utils\gc.h"
#include "Location.h"
#include "..\gameDef.h"
#include "..\entity\Entity.h"

class [[carlbeks::predecl, carlbeks::defineat("World.h")]] World;

/**
 * @note 所有的继承类都必须
 * @code friend class Game @endcode
 */
class Block : public IRenderable, public ITickable {
	friend class Garbage<Block>;
	friend class World;
	World* world = nullptr;
	BlockLocation location;

protected:
	Block(const BlockLocation& location) : location(location) { this->location.setWorld(0); }

public:
	[[nodiscard]] const BlockLocation& getLocation() const { return location; }
	virtual void onEnterWorld(World* world, WorldTransportReason reason) {}
	virtual void onExitWorld(World* world, WorldTransportReason reason) {}
	virtual void onRemove() { gc.submit<Block>(this); }
	/**
	 * 渲染方块影。方块影会覆盖所有的方块。
	 */
	virtual void renderShadow() const noexcept {}
	/**
	 * 渲染方块本体。渲染范围不应当超过方块占据的范围。
	 */
	void render(double tickDelta, QWORD tickRendering) const noexcept override = 0;

	/**
	 * @brief 调整一个实体与该方块交互时的速度。
	 * @param entity 目标实体
	 * @param position 实体当前判定到的位置
	 * @param velocity 可以直接进行、经过其他方块截短调整的速度
	 * @param rest 本次调整的原始速度
	 * @param currentRest 经过其他方块调整后的剩余速度
	 * @param side 碰撞方向
	 * @returns (bool) true - 进行了修改; false - 没有进行修改
	 * @attention 函数内方块状态不要发生变化。该函数只是试探性调整速度，且只进行速度的截短，并不是真正的碰撞交互。
	 */
	virtual bool adaptEntityVelocity(Entity& entity, const Vector2D& position, Vector2D& velocity, const Vector2D& rest, Vector2D& currentRest, CollidingSide side) const;
	virtual bool checkEntityOnGround(Entity& entity) const { return dEquals(entity.getLocation().getY() + entity.getBoundingBox().getBottom(), static_cast<double>(location.getY())); }

	[[nodiscard]] virtual Vector<RenderableString> getDescription() const {
		return Vector{
			L"\\#ffee0000<UnknownBlock>"_renderable,
			RenderableString(L"\\#ffee0000- " + location.toString())
		};
	}
};

class PureBarrierBlock final : public Block {
	friend class Game;
	unsigned int color = 0xffeeeeee;
	PureBarrierBlock(const BlockLocation& location) : Block(location) {}
	~PureBarrierBlock() override = default;

public:
	void render(double tickDelta, QWORD tickRendering) const noexcept override { renderer.fillWorldBlock(getLocation(), color); }
	void tick() noexcept(false) override {}
	void setColor(const unsigned int color) noexcept { this->color = color; }

	[[nodiscard]] Vector<RenderableString> getDescription() const override {
		return Vector{
			L"PureBarrierBlock"_renderable,
			RenderableString(L"- " + getLocation().toString()),
			RenderableString(L"- Color: \\#" + qwtowb16(color, 8) + L"#" + qwtowb16(color, 8))
		};
	}
};

class TestBarrierBlock final : public Block {
	friend class Game;
	TestBarrierBlock(const BlockLocation& location) : Block(location) {}
	~TestBarrierBlock() override = default;

public:
	void render(double tickDelta, QWORD tickRendering) const noexcept override { renderer.textureWorld(textureManager.getNullTexture(), getLocation().getPosition()); }
	void tick() noexcept(false) override {}

	[[nodiscard]] Vector<RenderableString> getDescription() const override {
		return Vector{
			L"TestBarrierBlock"_renderable,
			RenderableString(L"- " + getLocation().toString())
		};
	}
};

class TimedBarrierBlock final : public Block {
	friend class Game;
	unsigned int time = 50;
	TimedBarrierBlock(const BlockLocation& location) : Block(location) {}
	~TimedBarrierBlock() override = default;

public:
	void render(double tickDelta, QWORD tickRendering) const noexcept override;
	void tick() noexcept(false) override;

	[[nodiscard]] Vector<RenderableString> getDescription() const override {
		return Vector{
			L"TimedBarrierBlock"_renderable,
			RenderableString(L"- " + getLocation().toString()),
			RenderableString(L"- timeLeft: " + std::to_wstring(time))
		};
	}
};
