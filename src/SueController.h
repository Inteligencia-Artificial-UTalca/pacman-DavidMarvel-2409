#pragma once

#include "Controller.h"
#include <chrono>
class SueChaseState;
class SueScatterState;
class SueFrightenedState;

class SueFSM;
class SueController : public Controller
{
	std::shared_ptr<SueFSM> fsm;

public:
	SueController(std::shared_ptr<Character> character);
	virtual ~SueController();
	virtual Move getMove(const GameState &game) override;
};
