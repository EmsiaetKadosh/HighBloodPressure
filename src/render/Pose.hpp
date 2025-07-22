//
// Created by EmsiaetKadosh on 25-6-25.
//

#pragma once

#include "..\global.hpp"
#include "..\def.h"

enum class Action {
	None = 0, // 无动作，用于检测该组件是否存在
	Stand, Sit,
	Walk, Sprint, Jump, Dodge /* 闪避 */, Roll /* 翻滚 */,
	LightAttack, LightComboAttack, StringAttack, StrongAttack,
	Block, Teleport,
};

struct SkeletonInfo {
	double degree = 0;
	double length = 0;
};

class Skeleton : protected SkeletonInfo {
	friend class Model;
	friend class ModelManager;
	const String id;
	Skeleton* previous = nullptr; // nullptr -> head-skeleton

	void setPrevious(Skeleton* previous) noexcept { this->previous = previous; }

public:
	explicit Skeleton(String&& id) noexcept : id(std::move(id)) {}
	explicit Skeleton(const String& id) noexcept : id(id) {}

	[[nodiscard]] const String& getID() const noexcept { return id; }
	void rotate(const double degree) noexcept { this->degree = degree; }
};

using ModelOperator = Function<bool(Model&, Action)>;
using ModelCreator = Function<Map<String, Skeleton>()>;

class Model {
	friend class ModelProcessor;
	Map<String, Skeleton> skeletons {};
	String idModel {};
	Reference<ModelOperator> callback = nullptr;

	Model() = default;
	Model&& setSkeletons(Map<String, Skeleton>&& skeletons) && { return this->skeletons = std::move(skeletons), std::move(*this); }
	Model&& setIdModel(const String& idModel) && { return this->idModel = idModel, std::move(*this); }
	Model&& setCallback(Reference<ModelOperator>&& callback) && { return this->callback = std::move(callback), std::move(*this); }

public:
	[[nodiscard]] String getID() const noexcept { return idModel; }
	void move(const Action action) { callback(*this, action); }
};

class KeyFrame {
	friend class ModelActionScheduler;
	friend class ModelActionExecutor;
	Map<String, SkeletonInfo> skeletons;
	QWORD tick;

public:
	KeyFrame(const QWORD tick) noexcept : tick(tick) {}
	KeyFrame& set(const String& id, SkeletonInfo info) & noexcept;
	KeyFrame&& set(const String& id, SkeletonInfo info) && noexcept;
	KeyFrame& set(String&& id, SkeletonInfo info) & noexcept;
	KeyFrame&& set(String&& id, SkeletonInfo info) && noexcept;
};

class ModelActionScheduler {
	friend class ModelActionExecutor;
	List<KeyFrame> keyframes;

public:
	ModelActionScheduler& newKeyframe(QWORD tick, const Function<void(KeyFrame&)>& editor) & noexcept;
	ModelActionScheduler&& newKeyframe(QWORD tick, const Function<void(KeyFrame&)>& editor) && noexcept;
};

class ModelActionExecutor {
	struct ScheduleInfo : SkeletonInfo {
		QWORD tick = 0;
	};

	struct Schedule {
		Vector<ScheduleInfo> infos;
	};

	Map<String, Schedule> schedules;

public:
	ModelActionExecutor(const ModelActionScheduler& scheduler) noexcept;
	[[nodiscard]] SkeletonInfo get(const String& id, QWORD tick, double tickDelta) const noexcept;
};

class ModelProcessor {
	friend class ModelManager;
	String id;
	ModelOperator operatorCallback;
	ModelCreator creator;
	Map<String, ModelActionExecutor> actions {};
	ModelProcessor(const String& id) noexcept;

public:
	ModelOperator& getCallback() noexcept;
	ModelActionExecutor& getAction(const String& id) noexcept;
	[[nodiscard]] Model create() noexcept;
};

class ModelManager {
	friend class ModelProcessor;
	Map<String, ModelProcessor> models;
	ModelProcessor* nullProcessor = nullptr;
	ModelActionExecutor nullExecutor = ModelActionScheduler().newKeyframe(0, [](KeyFrame& frame) { frame.set(L"root", SkeletonInfo(0, 0)); });

public:
	ModelManager();
	ModelProcessor& getModel(const String& id) noexcept;
	/**
	 * 创建一个模型
	 * @param id 模型ID
	 * @param op 模型操作符
	 * @param cr 模型创建器。注意，返回的Map中必须要有一个ID为空字符串的根节点，且其他所有骨架必须直接或间接连接到根节点。根节点的长度、角度将被忽略
	 * @param actions 模型允许的所有动作，以及助性动作所采用的执行器
	 * @return 同ID的模型处理器，以及是否成功创建。如果失败，返回原有的模型处理器且不会按照参数创建新的模型处理器
	 */
	std::pair<ModelProcessor&, bool> createModel(const String& id, ModelOperator op, ModelCreator cr, Map<String, ModelActionExecutor>&& actions) noexcept;
};
