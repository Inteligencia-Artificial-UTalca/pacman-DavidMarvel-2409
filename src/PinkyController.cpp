#include "PinkyController.h"
PinkyInfo *PinkyInfo::_PinkyInfo = nullptr;

PinkyController::PinkyController(std::shared_ptr<Character> character) : Controller(character),
																		 root(std::make_shared<Selector>())
{
	auto frightenedFilter = std::make_shared<Filter>();
	auto scatterFilter = std::make_shared<Filter>();

	frightenedFilter->addCondition(std::make_shared<IsEdible>());
	frightenedFilter->addAction(std::make_shared<PinkyFrightened>());
	scatterFilter->addCondition(std::make_shared<PinkyTimeOut>());
	scatterFilter->addAction(std::make_shared<PinkyScatter>());

	root->addChild(frightenedFilter);
	root->addChild(scatterFilter);
	root->addChild(std::make_shared<PinkyChase>());
	root->addChild(std::make_shared<PinkyRandom>());
}

PinkyController::~PinkyController()
{
}

Move PinkyController::getMove(const GameState &game)
{
	PinkyInfo::getInfo()->_gameState = &game;
	PinkyInfo::getInfo()->_Character = character;
	root->tick();

	return PinkyInfo::getInfo()->_move;
}

Status IsEdible::update()
{
	auto character = PinkyInfo::getInfo()->_Character;
	auto ghost = dynamic_cast<Ghost *>(character.get());

	if (ghost != nullptr && ghost->isEdible())
	{
		return BH_SUCCESS;
	}
	return BH_FAILURE;
}

Status PinkyFrightened::update()
{
	auto game = PinkyInfo::getInfo()->_gameState;
	auto character = PinkyInfo::getInfo()->_Character;

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
	PinkyInfo::getInfo()->_move = m;
	return BH_SUCCESS;
}
Status PinkyRandom::update()
{
	auto game = PinkyInfo::getInfo()->_gameState;
	auto character = PinkyInfo::getInfo()->_Character;

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
	PinkyInfo::getInfo()->_move = m;
	return BH_SUCCESS;
}

Status PinkyChase::update()
{
	auto game = PinkyInfo::getInfo()->_gameState;
	auto character = PinkyInfo::getInfo()->_Character;

	auto pacmanpos_ = game->getPacmanPos();
	auto pacmandir_ = game->getPacmanDir();

	auto target = game->getMaze().getNodePos(pacmanpos_);
	switch (pacmandir_)
	{
	case UP:
		target.second -= 4;
		break;

	case DOWN:
		target.second += 4;
		break;

	case LEFT:
		target.first -= 4;
		break;

	case RIGHT:
		target.first += 4;
		break;

	default:
		break;
	}

	float min = 99999999;
	Move minmov = PASS;
	std::vector<Move> moves;

	if (character->getDirection() == PASS)
	{
		moves = game->getMaze().getPossibleMoves(character->getPos());
	}
	else
	{
		moves = game->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	for (auto move : moves)
	{
		if (move == PASS)
		{
			continue;
		}
		float distancia = euclid2(target, game->getMaze().getNodePos(game->getMaze().getNeighbour(character->getPos(), move)));
		if (distancia < min)
		{
			min = distancia;
			minmov = move;
		}
	}
	PinkyInfo::getInfo()->_move = minmov;
	return BH_SUCCESS;
}

PinkyTimeOut::PinkyTimeOut()
{
	lastTime = std::chrono::high_resolution_clock::now();
}
Status PinkyTimeOut::update()
{
	std::chrono::duration<float> transcurrido = std::chrono::high_resolution_clock::now() - lastTime;
	int time = (int)transcurrido.count();
	if (time % 27 < 7)
	{
		return BH_SUCCESS;
	}

	return BH_FAILURE;
}

PinkyScatter::PinkyScatter()
{
	target = std::make_pair(0, 0);
}

Status PinkyScatter::update()
{
	auto game = PinkyInfo::getInfo()->_gameState;
	auto character = PinkyInfo::getInfo()->_Character;

	std::vector<Move> moves;

	if (character->getDirection() == PASS)
	{
		moves = game->getMaze().getPossibleMoves(character->getPos());
	}
	else
	{
		moves = game->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	float min = 9999999;
	Move mejorMove = PASS;

	for (auto move : moves)
	{
		if (move == PASS)
		{
			continue;
		}
		auto nextPos = game->getMaze().getNodePos(game->getMaze().getNeighbour(character->getPos(), move));
		float distancia = euclid2(target, nextPos);
		if (distancia < min)
		{
			min = distancia;
			mejorMove = move;
		}
	}
	PinkyInfo::getInfo()->_move = mejorMove;
	return BH_SUCCESS;
}
