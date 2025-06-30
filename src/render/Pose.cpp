//
// Created by EmsiaetKadosh on 25-6-25.
//

#include "Pose.hpp"

ModelManager::ModelManager() {
	registerModel(L"HumanLike", [](Model& skeletons, const Action action) -> bool {
		switch (action) {
			case Action::None:
				break;
			case Action::Stand:
				break;
			case Action::Sit:
				break;
			case Action::Walk:
				break;
			case Action::Sprint:
				break;
			case Action::Jump:
				break;
			case Action::Dodge:
				break;
			case Action::Roll:
				break;
			case Action::LightAttack:
				break;
			case Action::LightComboAttack:
				break;
			case Action::StringAttack:
				break;
			case Action::StrongAttack:
				break;
			case Action::Block:
				break;
			case Action::Teleport:
				break;
		}
		return true;
	});
	initializerChecker.registerModule(L"ModelManager");
}

bool ModelManager::registerModel(String id, ModelProcessor callback) noexcept {
	if (models.contains(id)) return false; // 已存在
	models.emplace(std::move(id), std::move(callback));
	return true;
}

ModelProcessor& ModelManager::getModel(const String& id) noexcept {
	if (const auto iter = models.find(id); iter != models.cend()) return iter->second;
	return nullProcessor;
}

inline const Skeleton Skeleton::null = { L"nullptr" };
