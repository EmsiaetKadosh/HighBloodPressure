//
// Created by EmsiaetKadosh on 25-3-21.
//

#pragma once

#include "..\Game.h"
#include "Entity.h"

/**
 * 《高血压》玩家设计
 * 血压条初始上限100，可因其他因素而增加减少。上限变化时，值保持与上限的比例不变。
 * 上限20%以下时为低血压状态，80%以上时为高血压状态。
 * 【被】【固】：血压越高，玩家移动速度、伤害越高，跳跃能力越好。反则反之。
 * 【被】【固】：高血压时，玩家血压越高，受持续高血压伤害越高；低血压时，玩家血压越低，受持续低血压伤害越高。
 * 【被】浴血奋战：玩家受伤时，血压会升高，血压伤害不会导致血压变化。
 * 【被】静如止水：平常状态下，玩家的血压会自然降低。
 * 【被】进击号角：玩家操作时，血压会少许升高。玩家输入操作越频繁，血压升高越快。这一升高大约恰好抵消自然降低。
 * 【被】胸有成竹：当玩家输入操作因动作前后摇而无法立即执行时，会额外升高血压。
 * 【被】心急如焚：当玩家在前后摇期间频繁输入覆盖先前无法立即执行的动作时，血压会急速升高。
 * 【被】信仰之跃：血压高时，可以消耗60%的血压获得一段强力的二段跳，附带眩晕、伤害、破坏。
 * 【被】涣屠之咏：高血压时，玩家击杀敌人会使血压降低到40%，并回复体力。
 * 【被】往生狂啸：低血压时，受到攻击时，若攻击致死，则锁血1点，立即进入高血压、3s内不受血压伤害、立即反击。
 * 【被】涌泉之力：血压50%以上时，可以蹬墙跳。每跳一次会导致血压升高10。
 * 【被】耐力无尽：玩家持续奔跑时（中途可以有最多0.5s的停顿），取消【被】静如止水，跑的时间越长，血压越高，额外速度越快，但不会进入高血压。
 * 【被】放浪追击：血压70%以上时，若奔跑速度不小于[?]，下一次攻击范围暴击、破坏。
 * 【被】背水一战：血量越低，血压自然降低速度越慢，高血压伤害越低，低血压伤害越高。
 * 【主】背水一战：伤害+50%、受伤+20%、攻击附带10%吸血，若持续0.6s没有打出过伤害，取消【被】背水一战（若有），【被】【固】的高血压伤害翻倍。清除周围的怪物后，该效果解除。
 * 【被】猪突猛进：血压50%以上时，若处于奔跑状态不少于0.5s，下一次闪避附带冲撞、眩晕，按命中和伤害升高血压。
 * 【主】猪突猛进：血压50%以上时，主动启动猪突猛进，在下一次闪避之前，取消【被】静如止水，改为按血压值额外增加奔跑速度，奔跑时少量消耗血压，不奔跑时快速增加血压
*/
class Player final : public Entity {
	char airJump = 1;

	Player(const Vector2D& location) : Entity(location) {
		boundingBox.setLeft(0.6);
		boundingBox.setRight(0.6);
		boundingBox.setTop(2.5);
		maxSpeed = 0.3;
	}

public:
	void tick() noexcept(false) override {
		updatePosition();
		accelerate = Vector2D();
		if (interactManager.getKey(VK_SPACE).wasPressedAndDeal()) {
			if (isOnGround()) accelerate.setY(-0.4);
			else if (airJump == 2) velocity.setY(-0.25), accelerate.setY(0), --airJump;
			else if (airJump == 1) {
				--airJump, velocity.setY(-0.4), accelerate.setY(0);
				if (interactManager.getKey('A').isPressed()) velocity.setX(nMin(-0.35, velocity.getX()));
				if (interactManager.getKey('D').isPressed()) velocity.setX(nMax(0.35, velocity.getX()));
			}
		}
		if (interactManager.getKey('A').isPressed()) accelerate.add(-0.02, 0);
		if (interactManager.getKey('D').isPressed()) accelerate.add(0.02, 0);
		if (accelerate.getX() != 0) accelerate.setX(nRange(accelerate.getX(), (-maxSpeed - velocity.getX()) * 0.2, (maxSpeed - velocity.getX()) * 0.2));
		Entity::tick();
	}

	void checkOnGround() noexcept override {
		Entity::checkOnGround();
		if (onGround) airJump = 2;
	}

	void render(const double tickDelta, const QWORD tickRendering) const noexcept override {
		if (renderer.getCamera().getRenderingTargetEntity() == this) {
			renderer.fillWorld(renderer.getCamera().getTargetPosition().add(boundingBox.getLeftTopOffset()), boundingBox.getWidth(), boundingBox.getHeight(), airJump == 2? 0xff44ee66 : airJump == 1 ? 0xffeeee66 : 0xffdd7755);
			return;
		}
		momentum.atomicAcquire();
		renderer.fillWorld(getLocation(tickDelta, tickRendering).getPosition().add(boundingBox.getLeftTopOffset()), boundingBox.getWidth(), boundingBox.getHeight(), airJump == 2? 0xff44ee66 : airJump == 1 ? 0xffeeee66 : 0xffdd7755);
		momentum.atomicRelease();
	}

	void onDamage(Damage&) override {}
	void onDeath() override {}

	static Player* create(const Vector2D& location) noexcept { return allocatedFor(new Player(location)); }
};
