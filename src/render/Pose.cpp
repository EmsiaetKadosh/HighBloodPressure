//
// Created by EmsiaetKadosh on 25-6-25.
//

#include "..\utils\math.h"
#include "Pose.hpp"

ModelProcessor::ModelProcessor(const String& id) noexcept : id(id), operatorCallback(modelManager.nullOperator) {}

ModelOperator& ModelProcessor::getCallback() noexcept {
	if (operatorCallback) return operatorCallback;
	return modelManager.nullOperator;
}

ModelActionExecutor& ModelProcessor::getAction(const String& id) noexcept {
	if (const auto iter = actions.find(id); iter != actions.end()) return iter->second;
	return modelManager.nullExecutor;
}

Model ModelProcessor::create() noexcept { return Model().setIdModel(id).setCallback(getCallback()).setSkeletons(creator()); }

KeyFrame& KeyFrame::set(const String& id, const SkeletonInfo info) & noexcept { return skeletons.emplace(id, info), *this; }
KeyFrame& KeyFrame::set(String&& id, const SkeletonInfo info) & noexcept { return skeletons.emplace(std::move(id), info), *this; }
KeyFrame&& KeyFrame::set(const String& id, const SkeletonInfo info) && noexcept { return std::move(set(id, info)); }
KeyFrame&& KeyFrame::set(String&& id, const SkeletonInfo info) && noexcept { return std::move(set(id, info)); }
ModelActionScheduler& ModelActionScheduler::newKeyframe(const QWORD tick, const Function<void(KeyFrame&)>& editor) & noexcept { return editor(keyframes.emplace_back(tick)), *this; }
ModelActionScheduler&& ModelActionScheduler::newKeyframe(const QWORD tick, const Function<void(KeyFrame&)>& editor) && noexcept { return std::move(newKeyframe(tick, editor)); }

ModelActionExecutor::ModelActionExecutor(const ModelActionScheduler& scheduler) noexcept {
	for (const auto& [tick, skeletons] : scheduler.keyframes)
		for (const auto& [id, info] : skeletons) {
			auto iter = schedules.find(id);
			if (iter == schedules.end()) iter = schedules.emplace(std::move(id), Schedule()).first;
			iter->second.infos.emplace_back(tick, info);
		}
}

SkeletonInfo ModelActionExecutor::get(const String& id, const QWORD tick, const double tickDelta) const noexcept {
	const auto iter = schedules.find(id);
	if (iter == schedules.end()) return { 0, 0 };
	const Vector<ScheduleInfo>& infos = iter->second.infos;
	if (infos.empty()) return { 0, 0 };
	ScheduleInfo target;
	target.tick = tick;
	const std::vector<ScheduleInfo>::const_iterator lower = std::lower_bound(infos.begin(), infos.end(), target, [](const std::pair<QWORD, SkeletonInfo>& a, const std::pair<QWORD, SkeletonInfo>& b) { return a.first < b.first; });
	ScheduleInfo left, right;
	if (lower == infos.begin()) left = right = infos.front();
	else if (lower == infos.end()) left = right = infos.back();
	else {
		right = *lower;
		left = *(lower - 1);
	}
	const double t = (static_cast<double>(tick - left.tick) + tickDelta) / static_cast<double>(right.tick - left.tick);
	return SkeletonInfo {
		.degree = nSlerp(left.degree, right.degree, t),
		.length = nSlerp(left.length, right.length, t)
	};
}

ModelManager::ModelManager() {
	createModel(L"Null", [](Model&, Action) -> bool { return false; }, []() -> Map<String, Skeleton> { return { std::make_pair(String(L"root"), Skeleton(L"root")) }; }, { std::make_pair(String(L"default"), nullExecutor) });
	createModel(
		L"HumanLike",
		[](Model& model, Action action) -> bool {
			model.g'etID()
		},
		[]() -> Map<String, Skeleton> {},
		{
			std::make_pair(String(L"default"), nullExecutor),
		});
	initializerChecker.registerModule(L"ModelManager");
}

ModelProcessor& ModelManager::getModel(const String& id) noexcept {
	if (const auto iter = models.find(id); iter != models.cend()) return iter->second;
	return *nullProcessor;
}

std::pair<ModelProcessor&, bool> ModelManager::createModel(const String& id, ModelOperator op, ModelCreator cr, Map<String, ModelActionExecutor>&& actions) noexcept {
	if (const auto iter = models.find(id); iter != models.cend()) return { iter->second, false };
	ModelProcessor& model = models.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple()).first->second;
	model.operatorCallback = std::move(op);
	model.creator = std::move(cr);
	model.actions = std::move(actions);
	return { model, true };
}
