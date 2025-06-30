//
// Created by EmsiaetKadosh on 25-6-25.
//

#pragma once

#include "..\global.hpp"
#include "..\def.h"
#include "..\utils\math.h"

enum class Action {
	None = 0, // 无动作，用于检测该组件是否存在
	Stand, Sit,
	Walk, Sprint, Jump, Dodge /* 闪避 */, Roll /* 翻滚 */,
	LightAttack, LightComboAttack, StringAttack, StrongAttack,
	Block, Teleport,
};

class Skeleton {
	friend class Model;
	static const Skeleton null;
	const String id;
	Skeleton* previous = nullptr; // nullptr -> head-skeleton
	Vector2D direction;

	void setPrevious(Skeleton* previous) noexcept { this->previous = previous; }

public:
	Skeleton(String&& id) noexcept : id(id) {}

	[[nodiscard]] const String& getID() const noexcept { return id; }
	void toDirection() const noexcept {}
};

using ModelProcessor = Function<bool(Model&, Action)>;

class ModelManager {
	Map<String, ModelProcessor> models;
	ModelProcessor nullProcessor;

public:
	ModelManager();
	bool registerModel(String id, ModelProcessor callback) noexcept;
	ModelProcessor& getModel(const String& id) noexcept;
};

class Model {
	Map<String, Skeleton> skeletons;
	String callbackID;
	ModelProcessor& callback;

public:
	Model(String&& idCallback) : callbackID(std::move(idCallback)), callback(modelManager.getModel(callbackID)) {}
	void move(const Action action) { callback(*this, action); }
};
