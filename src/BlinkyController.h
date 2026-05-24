#pragma once

#include "Controller.h"
#include <chrono>
class BlinkyChaseState;
class BlinkyScatterState;
class BlinkyFrightenedState;

class BlinkyFSM;

class BlinkyController : public Controller
{
	std::shared_ptr<BlinkyFSM> fsm;

public:
	BlinkyController(std::shared_ptr<Character> character);
	virtual ~BlinkyController();
	virtual Move getMove(const GameState &game) override;
};
