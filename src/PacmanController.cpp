#include "PacmanController.h"
#include <SDL2/SDL.h>

PacmanController::PacmanController(std::shared_ptr<Character> character) : Controller(character)
{
}

PacmanController::~PacmanController()
{
}
Move PacmanController::getClosestMove(const GameState &game, std::pair<int, int> target) const
{
	float minDist = 9999999.0f;
	Move minMove = character->getDirection();
	std::vector<Move> moves = game.getMaze().getPossibleMoves(character->getPos());
	for (Move m : moves)
	{
		int vecino = game.getMaze().getNeighbour(character->getPos(), m);
		if (vecino < 0)
			continue;
		auto vecinoCoords = game.getMaze().getNodePos(vecino);
		int sqDist = euclid2(vecinoCoords, target);
		if (sqDist < minDist)
		{
			minDist = sqDist;
			minMove = m;
		}
	}
	return minMove;
}
Move PacmanController::getFarthestMove(const GameState &game, std::pair<int, int> target) const
{
	int maxDist = -1;
	Move maxMove = character->getDirection();
	std::vector<Move> moves = game.getMaze().getPossibleMoves(character->getPos());
	for (Move m : moves)
	{
		int vecino = game.getMaze().getNeighbour(character->getPos(), m);
		if (vecino < 0)
			continue;
		auto vecinoCoords = game.getMaze().getNodePos(vecino);
		int sqDist = euclid2(vecinoCoords, target);
		if (sqDist > maxDist)
		{
			maxDist = sqDist;
			maxMove = m;
		}
	}
	return maxMove;
}

float PacmanController::getDistanceToGhost(const GameState &game, int g) const
{
	return sqrt(euclid2(game.getMaze().getNodePos(character->getPos()),
						game.getMaze().getNodePos(game.getGhostsPos(g))));
}
Move PacmanController::getMove(const GameState &game)
{

	// para cerrar la ventana
	SDL_Event e;
	if (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT ||
			(e.type == SDL_KEYDOWN &&
			 (e.key.keysym.sym == SDLK_ESCAPE ||
			  e.key.keysym.sym == SDLK_q)))
		{
			SDL_Quit();
			exit(0);
		}
	}

	int pacmanNode = character->getPos();
	auto pacmanCoords = game.getMaze().getNodePos(pacmanNode);

	std::vector<std::pair<int, int>> ghostPositions;
	for (int i = 0; i < 4; i++)
	{
		ghostPositions.push_back(game.getMaze().getNodePos(game.getGhostsPos(i)));
	}
	std::vector<bool> ghostsEdible;
	for (int i = 0; i < 4; i++)
	{
		ghostsEdible.push_back(game.isGhostEdible(i));
	}
	float fear = 0.0f;
	// Move escapeMove = PASS;

	for (int i = 0; i < 4; i++)
	{
		if (!ghostsEdible[i])
		{
			float dist = getDistanceToGhost(game, i);
			float tempFear = 1.0f - 1.0f / (1.0f + exp(-0.2f * (20.0f - dist)));
			if (tempFear > fear)
			{
				fear = tempFear;
				// escapeMove = getFarthestMove(game, ghostPositions[i]);
			}
		}
	}

	auto pillPositions = game.getMaze().getPillPositions();
	auto powerPillPositions = game.getMaze().getPowerPillPositions();
	float collectUtility = 0.0f;
	float powerUtility = 0.0f;
	Move collectMove = PASS;
	Move powerMove = PASS;
	float closestPillDist = 999999.0f;
	std::pair<int, int> closestPill;
	float closestPowerDist = 999999.0f;
	std::pair<int, int> closestPowerPill;
	if (!pillPositions.empty())
	{
		for (auto &pillPos : pillPositions)
		{
			float dist = sqrt(euclid2(pacmanCoords, pillPos));
			if (dist < closestPillDist)
			{
				closestPillDist = dist;
				closestPill = pillPos;
			}
		}
		collectUtility = 1.0f / (1.0f + closestPillDist);
		collectMove = getClosestMove(game, closestPill);
	}

	if (!powerPillPositions.empty())
	{
		for (auto &powerPos : powerPillPositions)
		{
			float dist = sqrt(euclid2(pacmanCoords, powerPos));

			if (dist < closestPowerDist)
			{
				closestPowerDist = dist;
				closestPowerPill = powerPos;
			}
		}

		powerUtility = fear * (1.0f / (1.0f + closestPowerDist));
		powerMove = getClosestMove(game, closestPowerPill);
	}
	std::vector<Move> possibleMoves = game.getMaze().getPossibleMoves(pacmanNode);
	float safestUtility = -1.0f;
	Move safestMove = PASS;
	for (Move m : possibleMoves)
	{
		int vecino = game.getMaze().getNeighbour(pacmanNode, m);
		if (vecino < 0)
			continue;
		auto vecinoCoord = game.getMaze().getNodePos(vecino);
		float danger = 0;
		for (int i = 0; i < 4; i++)
		{
			if (!ghostsEdible[i])
			{
				float ghostDist = sqrt(euclid2(vecinoCoord, ghostPositions[i]));
				float ghostDanger = 1.0f / (1.0f + ghostDist);
				if (ghostDanger > danger)
				{
					danger = ghostDanger;
				}
			}
		}
		if (game.getMaze().isDeadEnd(vecino))
		{
			danger += 0.4f;
			danger = std::min(danger, 1.0f);
		}
		float safety = 1.0f - danger;
		if (safety > safestUtility)
		{
			safestUtility = safety;
			safestMove = m;
		}
	}

	float cautiousCollectUtility = collectUtility * safestUtility;

	if (fear > 0.4f && powerUtility > cautiousCollectUtility)
	{
		return powerMove;
	}
	else if (fear > cautiousCollectUtility)
	{
		return safestMove;
	}
	else
	{
		return collectMove;
	}
	// return PASS;
}
