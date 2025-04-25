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

class Block : public IRenderable, public ITickable {
	friend class Garbage<Block>;
	friend class World;
	World* world = nullptr;
	BlockLocation location;

public:
	Block(const BlockLocation& location) : location(location) { this->location.setWorld(0); }
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
	virtual bool adaptEntityVelocity(Entity& entity, const Vector2D& position, Vector2D& velocity, const Vector2D& rest, Vector2D& currentRest, const CollidingSide side) const {
		Vector2D adapted = velocity;
		switch (side) {
			case CollidingSide::LEFT_TOP:
			case CollidingSide::LEFT_BOTTOM:
			case CollidingSide::LEFT: {
				adapted.extendValueX(location.getX() - entity.getBoundingBox().getRight() - position.getX() /* - EpsilonValue */);
				currentRest.strictSelect((rest - adapted).setX(0));
				if (adapted.lengthManhattan() >= velocity.lengthManhattan()) return false;
				return velocity = adapted, true;
			}
			case CollidingSide::TOP: {
				adapted.extendValueY(location.getY() - entity.getBoundingBox().getBottom() - position.getY() /* - EpsilonValue */);
				currentRest.strictSelect((rest - adapted).setY(0));
				if (adapted.lengthManhattan() >= velocity.lengthManhattan()) return false;
				return velocity = adapted, true;
			}
			case CollidingSide::RIGHT_TOP:
			case CollidingSide::RIGHT_BOTTOM:
			case CollidingSide::RIGHT: {
				adapted.extendValueX(location.getX() + 1 + entity.getBoundingBox().getLeft() - position.getX() /* + EpsilonValue */);
				currentRest.strictSelect((rest - adapted).setX(0));
				if (adapted.lengthManhattan() >= velocity.lengthManhattan()) return false;
				return velocity = adapted, true;
			}
			case CollidingSide::BOTTOM: {
				adapted.extendValueY(location.getY() + 1 + entity.getBoundingBox().getTop() - position.getY() /* + EpsilonValue */);
				currentRest.strictSelect((rest - adapted).setY(0));
				if (adapted.lengthManhattan() >= velocity.lengthManhattan()) return false;
				return velocity = adapted, true;
			}
			default: // COVER此处也不做处理，既然穿过来了那就穿过来了吧
				break;
		}
		return false;
	}

	virtual Vector<RenderableString> getDescription() const { return Vector{L"\\#ffee0000<UnknownBlock>"_renderable}; }
	virtual bool checkEntityOnGround(Entity& entity) const { return dEquals(entity.getLocation().getY() + entity.getBoundingBox().getBottom(), static_cast<double>(location.getY())); }
};

class PureBarrierBlock final : public Block {
	unsigned int color = 0xffeeeeee;
	PureBarrierBlock(const BlockLocation& location) : Block(location) {}
	~PureBarrierBlock() override = default;

public:
	void render(double tickDelta, QWORD tickRendering) const noexcept override { renderer.fillWorldBlock(getLocation(), color); }
	void tick() noexcept(false) override {}
	void setColor(const unsigned int color) noexcept { this->color = color; }
	static PureBarrierBlock* create(const BlockLocation& location) { return allocatedFor(new PureBarrierBlock(location)); }
	Vector<RenderableString> getDescription() const override { return Vector{L"PureBarrierBlock"_renderable, RenderableString(L"- Color: \\#" + qwtowb16(color, 8) + L"#" + qwtowb16(color, 8))}; }
};

class TestBarrierBlock final : public Block {
	TestBarrierBlock(const BlockLocation& location) : Block(location) {}
	~TestBarrierBlock() override = default;

public:
	void render(double tickDelta, QWORD tickRendering) const noexcept override { renderer.fillWorldBlock(getLocation(), 0xffee0000); }
	void tick() noexcept(false) override {}

	bool adaptEntityVelocity(Entity& entity, const Vector2D& position, Vector2D& velocity, const Vector2D& rest, Vector2D& currentRest, const CollidingSide side) const override {
		Logger.trace(getLocation().toString() + L" adapting velocity");
		return Block::adaptEntityVelocity(entity, position, velocity, rest, currentRest, side);
	}

	static TestBarrierBlock* create(const BlockLocation& location) { return allocatedFor(new TestBarrierBlock(location)); }
	Vector<RenderableString> getDescription() const override { return Vector{L"TestBarrierBlock"_renderable}; }
};

class TimedBarrierBlock final : public Block {
	TimedBarrierBlock(const BlockLocation& location) : Block(location) {}
	~TimedBarrierBlock() override = default;

	unsigned int time = 50;

public:
	void render(double tickDelta, QWORD tickRendering) const noexcept override;
	void tick() noexcept(false) override;

	bool adaptEntityVelocity(Entity& entity, const Vector2D& position, Vector2D& velocity, const Vector2D& rest, Vector2D& currentRest, const CollidingSide side) const override {
		Logger.trace(getLocation().toString() + L" adapting velocity");
		return Block::adaptEntityVelocity(entity, position, velocity, rest, currentRest, side);
	}

	static TimedBarrierBlock* create(const BlockLocation& location) { return allocatedFor(new TimedBarrierBlock(location)); }
	Vector<RenderableString> getDescription() const override { return Vector{L"TimedBarrierBlock"_renderable, RenderableString(L"- timeLeft: " + std::to_wstring(time))}; }
};
