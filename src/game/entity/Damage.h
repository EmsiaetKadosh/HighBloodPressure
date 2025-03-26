//
// Created by EmsiaetKadosh on 25-3-21.
//

#pragma once

#include "..\..\def.h"

class [[carlbeks::predecl, carlbeks::defineat("Entity.h")]] Entity;

struct DamageTypeEnum {
	enum : unsigned char {
		NONE, // 无伤害
		PHYSICAL_DAMAGE, // 物理伤害
		MAGICAL_DAMAGE, // 魔法伤害
		TRUE_DAMAGE, // 真实伤害
		DENY_DAMAGE, // 否定伤害
		RETURN_DAMAGE, // 回敬伤害
		LOW_BLOOD_PRESSURE_DAMAGE, // 低压伤害
		HIGH_BLOOD_PRESSURE_DAMAGE, // 高压伤害
		SYSTEMATIC_DAMAGE, // 系统伤害
	} value;

	static constexpr QWORD elementCount() { return SYSTEMATIC_DAMAGE + 1; }
};

struct DamageFormEnum {
	enum : unsigned char {
		NONE, // 无伤害
		COMMON_MELEE_DAMAGE, // 近战伤害
		COMMON_REMOTE_DAMAGE, // 远程伤害
		COMMON_PROJECTILE_DAMAGE, // 弹射物伤害
		SKILL_MELEE_DAMAGE, // 技能近战
		SKILL_RANGED_DAMAGE, // 技能远程
		SKILL_PROJECTILE_DAMAGE, // 技能弹射物
		CONTINUOUS_DAMAGE, // 持续伤害
		EXECUTE_DAMAGE, // 处决伤害
	} value;

	static constexpr QWORD elementCount() { return EXECUTE_DAMAGE + 1; }
};

struct Damage {
	double damages[DamageTypeEnum::elementCount()]{};
	DamageFormEnum form;
	Damage(const DamageFormEnum form) : form(form) {}
	Damage& physicalDamage(const double value) noexcept { return damages[DamageTypeEnum::PHYSICAL_DAMAGE] = value, *this; }
	Damage& magicalDamage(const double value) noexcept { return damages[DamageTypeEnum::MAGICAL_DAMAGE] = value, *this; }
	Damage& trueDamage(const double value) noexcept { return damages[DamageTypeEnum::TRUE_DAMAGE] = value, *this; }
	Damage& denyDamage(const double value) noexcept { return damages[DamageTypeEnum::DENY_DAMAGE] = value, *this; }
	Damage& returnDamage(const double value) noexcept { return damages[DamageTypeEnum::RETURN_DAMAGE] = value, *this; }
	Damage& lowBloodPressure(const double value) noexcept { return damages[DamageTypeEnum::LOW_BLOOD_PRESSURE_DAMAGE] = value, *this; }
	Damage& highBloodPressure(const double value) noexcept { return damages[DamageTypeEnum::HIGH_BLOOD_PRESSURE_DAMAGE] = value, *this; }
	Damage& systematicDamage(const double value) noexcept { return damages[DamageTypeEnum::SYSTEMATIC_DAMAGE] = value, *this; }
	[[nodiscard]] double getPhysicalDamage() const noexcept { return damages[DamageTypeEnum::PHYSICAL_DAMAGE]; }
	[[nodiscard]] double getMagicalDamage() const noexcept { return damages[DamageTypeEnum::MAGICAL_DAMAGE]; }
	[[nodiscard]] double getTrueDamage() const noexcept { return damages[DamageTypeEnum::TRUE_DAMAGE]; }
	[[nodiscard]] double getDenyDamage() const noexcept { return damages[DamageTypeEnum::DENY_DAMAGE]; }
	[[nodiscard]] double getReturnDamage() const noexcept { return damages[DamageTypeEnum::RETURN_DAMAGE]; }
	[[nodiscard]] double getLowBloodPressure() const noexcept { return damages[DamageTypeEnum::LOW_BLOOD_PRESSURE_DAMAGE]; }
	[[nodiscard]] double getHighBloodPressure() const noexcept { return damages[DamageTypeEnum::HIGH_BLOOD_PRESSURE_DAMAGE]; }
	[[nodiscard]] double getSystematicDamage() const noexcept { return damages[DamageTypeEnum::SYSTEMATIC_DAMAGE]; }
};
