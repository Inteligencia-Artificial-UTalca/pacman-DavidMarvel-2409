#include "BlinkyController.h"
#include "GameState.h"
#include "BehaviorTree.h"
#include <limits>

BlinkyController::BlinkyController(std::shared_ptr<Character> character) : Controller(character)
{
}

BlinkyController::~BlinkyController()
{
}
Move BlinkyController::getMove(const GameState &game)
{
    auto ghost = dynamic_cast<Ghost *>(character.get());
    auto pacmanPos = game.getMaze().getNodePos(game.getPacmanPos());

    std::vector<Move> moves;

    if (character->getDirection() == PASS)
    {
        moves = game.getMaze().getPossibleMoves(character->getPos());
    }
    else
    {
        moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }
    Move bestMove = PASS;

    if (ghost->isEdible())
    {
        float maxDist = -1;
        for (auto move : moves)
        {
            int nextNode = game.getMaze().getNeighbour(character->getPos(), move);
            auto nextPos = game.getMaze().getNodePos(nextNode);
            float dist = euclid2(nextPos, pacmanPos);
            if (dist > maxDist)
            {
                maxDist = dist;
                bestMove = move;
            }
        }
    }
    else
    {
        float minDist = std::numeric_limits<float>::max();
        for (auto move : moves)
        {
            int nextNode = game.getMaze().getNeighbour(character->getPos(), move);
            auto nextPos = game.getMaze().getNodePos(nextNode);
            float dist = euclid2(nextPos, pacmanPos);
            if (dist < minDist)
            {
                minDist = dist;
                bestMove = move;
            }
        }
    }
    return bestMove;
}
