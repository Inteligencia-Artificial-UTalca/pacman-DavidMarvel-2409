#include "InkyController.h"
#include "Ghost.h"
#include <cstdlib>
InkyInfo *InkyInfo::_InkyInfo = nullptr;

InkyController::InkyController(std::shared_ptr<Character> character) : Controller(character), root(std::make_shared<Selector>())
{
	auto frightenedFilter = std::make_shared<Filter>();
	auto scatterFilter = std::make_shared<Filter>();

	frightenedFilter->addCondition(std::make_shared<InkyIsEdible>());
	frightenedFilter->addAction(std::make_shared<InkyFrightened>());

	scatterFilter->addCondition(std::make_shared<InkyTimeOut>());
	scatterFilter->addAction(std::make_shared<InkyScatter>());

	root->addChild(frightenedFilter);
	root->addChild(scatterFilter);
	root->addChild(std::make_shared<InkyChase>());
	root->addChild(std::make_shared<InkyRandom>());
}

InkyController::~InkyController()
{
}

Move InkyController::getMove(const GameState &game)
{
	InkyInfo::getInfo()->_gameState = &game;
	InkyInfo::getInfo()->_Character = character;

	root->tick();
	return InkyInfo::getInfo()->_move;
}
Status InkyChase::update()
{
	auto game = InkyInfo::getInfo()->_gameState;
	auto character = InkyInfo::getInfo()->_Character;
	auto pacPos = game->getMaze().getNodePos(game->getPacmanPos());
	auto pacDir = game->getPacmanDir();

	auto target = pacPos;

	switch (pacDir)
	{
	case UP:
		target.second -= 2;
		break;

	case DOWN:
		target.second += 2;
		break;

	case LEFT:
		target.first -= 2;
		break;

	case RIGHT:
		target.first += 2;
		break;

	default:
		break;
	}

	auto blinkyPos = game->getMaze().getNodePos(game->getGhostsPos(0));

	target.first = target.first * 2 - blinkyPos.first;
	target.second = target.second * 2 - blinkyPos.second;

	float min = 99999999;
	Move minMove = PASS;

	std::vector<Move> moves;

	if (character->getDirection() == PASS)
	{
		moves = game->getMaze().getPossibleMoves(character->getPos());
	}
	else
	{
		moves = game->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	if (moves.empty())
	{
		return BH_FAILURE;
	}

	for (auto move : moves)
	{
		if (move == PASS)
			continue;
		auto nextPos = game->getMaze().getNodePos(game->getMaze().getNeighbour(character->getPos(), move));
		float dist = euclid2(target, nextPos);

		if (dist < min)
		{
			min = dist;
			minMove = move;
		}
	}
	InkyInfo::getInfo()->_move = minMove;
	return BH_SUCCESS;
}

InkyScatter::InkyScatter()
{
	target = std::make_pair(108, 108);
}

Status InkyFrightened::update()
{
	auto game = InkyInfo::getInfo()->_gameState;
	auto character = InkyInfo::getInfo()->_Character;

	std::vector<Move> moves;

	if (character->getDirection() == PASS)
	{
		moves = game->getMaze().getPossibleMoves(character->getPos());
	}
	else
	{
		moves = game->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	if (moves.empty())
	{
		return BH_FAILURE;
	}

	Move m = moves[rand() % moves.size()];

	InkyInfo::getInfo()->_move = m;

	return BH_SUCCESS;
}

Status InkyScatter::update()
{
	auto game = InkyInfo::getInfo()->_gameState;
	auto character = InkyInfo::getInfo()->_Character;

	std::vector<Move> moves;

	if (character->getDirection() == PASS)
	{
		moves = game->getMaze().getPossibleMoves(character->getPos());
	}
	else
	{
		moves = game->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	if (moves.empty())
	{
		return BH_FAILURE;
	}
	
	float min = 9999999;
	Move bestMove = PASS;

	for (auto move : moves)
	{
		if (move == PASS)
			continue;

		auto nextPos = game->getMaze().getNodePos(game->getMaze().getNeighbour(character->getPos(), move));

		float dist = euclid2(target, nextPos);

		if (dist < min)
		{
			min = dist;
			bestMove = move;
		}
	}

	InkyInfo::getInfo()->_move = bestMove;
	return BH_SUCCESS;
}

Status InkyIsEdible::update()
{
	auto character = InkyInfo::getInfo()->_Character;
	auto ghost = dynamic_cast<Ghost *>(character.get());

	if (ghost != nullptr && ghost->isEdible())
	{
		return BH_SUCCESS;
	}

	return BH_FAILURE;
}

Status InkyRandom::update()
{
	auto game = InkyInfo::getInfo()->_gameState;
	auto character = InkyInfo::getInfo()->_Character;

	std::vector<Move> moves;

	if (character->getDirection() == PASS)
	{
		moves = game->getMaze().getPossibleMoves(character->getPos());
	}
	else
	{
		moves = game->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	if (moves.empty())
	{
		return BH_FAILURE;
	}

	Move m = moves[rand() % moves.size()];

	InkyInfo::getInfo()->_move = m;

	return BH_SUCCESS;
}

InkyTimeOut::InkyTimeOut()
{
	lastTime = std::chrono::high_resolution_clock::now();
}

Status InkyTimeOut::update()
{
	std::chrono::duration<float> transcurrido = std::chrono::high_resolution_clock::now() - lastTime;

	int time = (int)transcurrido.count();

	if (time % 27 < 7)
	{
		return BH_SUCCESS;
	}

	return BH_FAILURE;
}