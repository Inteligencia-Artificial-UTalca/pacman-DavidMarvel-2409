#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include <chrono>

class InkyInfo
{
	static InkyInfo *_InkyInfo;
	InkyInfo() {}

public:
	static InkyInfo *getInfo()
	{
		if (_InkyInfo == nullptr)
			_InkyInfo = new InkyInfo();

		return _InkyInfo;
	}
	const GameState *_gameState;
	Move _move;
	std::shared_ptr<Character> _Character;
};

class InkyController : public Controller
{
	std::shared_ptr<Composite> root;

public:
	InkyController(std::shared_ptr<Character> character);
	virtual ~InkyController();
	virtual Move getMove(const GameState &game) override;
};

class InkyChase : public Behavior
{
public:
	virtual Status update() override;
};

class InkyFrightened : public Behavior
{
public:
	virtual Status update() override;
};

class InkyRandom : public Behavior
{
public:
	virtual Status update() override;
};

class InkyScatter : public Behavior
{
private:
	std::pair<int, int> target;

public:
	InkyScatter();
	virtual Status update() override;
};

class InkyTimeOut : public Behavior
{
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;

public:
	InkyTimeOut();
	virtual Status update() override;
};

class InkyIsEdible : public Behavior
{
public:
	virtual Status update() override;
};
