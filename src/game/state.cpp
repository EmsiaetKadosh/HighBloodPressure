//
// Created by EmsiaetKadosh on 2026/2/8.
//

#include <set>
#include "state.hpp"
#include "src\main.hpp"
#include "src\using.hpp"

struct LessGameState {
	using is_transparent = unsigned int;
	[[nodiscard]] bool operator()(const GameState& left, const GameState& right) const noexcept { return left.id < right.id; }
	[[nodiscard]] bool operator()(const unsigned int left, const GameState& right) const noexcept { return left < right.id; }
	[[nodiscard]] bool operator()(const GameState& left, const unsigned int right) const noexcept { return left.id < right; }
};

class GameStateManager::Impl {
public:
	Set<GameState> states = Set<GameState>();
	GameStateManager* manager;
	unsigned int id = 0;

	Impl(GameStateManager* manager) noexcept : manager(manager) {}

	~Impl() noexcept = default;

	GameStateEntry registerState(String&& name) noexcept {
		try {
			states.emplace(std::move(name), ++id);
		}
		catch (std::bad_alloc&) { /* TODO(EmsiaetKadosh): Report */ }
		catch (std::exception& e) { /* TODO(EmsiaetKadosh): Report */ }
		return GameStateEntry(manager, nullptr);
	}

	void push(const GameState& state) const noexcept(false) {}
	void pop(const GameState& state) const noexcept(false) {}
};

void GameStateEntry::push() const noexcept { manager->push(*this); }
void GameStateEntry::pop() const noexcept { manager->pop(*this); }
GameStateManager::GameStateManager() noexcept : impl(new Impl) {}
GameStateManager::~GameStateManager() noexcept { delete impl; }
GameStateEntry GameStateManager::registerState(String&& name) const noexcept { return impl->registerState(std::move(name)); }
void GameStateManager::push(const GameStateEntry& entry) const noexcept(false) { impl->push(*entry.state); }
void GameStateManager::pop(const GameStateEntry& entry) const noexcept(false) { impl->pop(*entry.state); }

