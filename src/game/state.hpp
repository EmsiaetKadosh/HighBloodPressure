//
// Created by EmsiaetKadosh on 2026/2/8.
//

#pragma once

#include <string>
#include "src\using.hpp"

class GameState {
	friend class GameStateManager;
	friend struct LessGameState;
	String name;
	unsigned long long pushCounter =0 ;
	const unsigned int id;
public:
	GameState(String&& name, const unsigned int id) noexcept : name(std::move(name)), id(id) {}
};

class GameStateEntry {
	friend class GameStateManager;
	GameState* state;
	GameStateManager* manager;
	GameStateEntry(GameStateManager* manager, GameState* state) noexcept : state(state), manager(manager) {}
	[[nodiscard]] operator bool() const noexcept { return state; }
	[[nodiscard]] bool operator!() const noexcept { return !state; }

public:
	void push() const noexcept;
	void pop() const noexcept;
};

class GameStateManager {
	class Impl;
	Impl* impl;

public:
	GameStateManager() noexcept;
	~GameStateManager() noexcept;
	/**
	 * @brief
	 * @param name 要注册的状态名称
	 * @return
	 */
	GameStateEntry registerState(String&& name) const noexcept;
	void push(const GameStateEntry&) const noexcept(false);
	void pop(const GameStateEntry&) const noexcept(false);
};
