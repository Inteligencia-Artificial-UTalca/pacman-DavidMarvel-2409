#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include <chrono>

class PinkyInfo
{
	static PinkyInfo *_PinkyInfo;
	PinkyInfo() {}

public:
	static PinkyInfo* getInfo(){
		if(_PinkyInfo == nullptr) _PinkyInfo = new PinkyInfo();
		return _PinkyInfo;
	}
	const GameState * _gameState;
	Move _move;
	std::shared_ptr<Character> _Character;
};

class PinkyController : public Controller
{
    std::shared_ptr<Composite> root;
public:
	PinkyController(std::shared_ptr<Character> character);
	virtual ~PinkyController();
	virtual Move getMove(const GameState &game) override;
};


class PinkyChase : public Behavior{
public:
    virtual Status update() override;

};
class PinkyFrightened : public Behavior{
public:
    virtual Status update() override;

};

class PinkyRandom : public Behavior {
public:
    virtual Status update() override;
};

class IsEdible : public Behavior {
public:
    virtual Status update() override;
};
