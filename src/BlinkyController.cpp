#include "BlinkyController.h"
#include "GameState.h"
#include "BehaviorTree.h"
#include <limits>

class ChasePacman : public Behavior
{
private:
    const GameState &game;
    int ghostIndex;
    // std::shared_ptr<Character> ghost;
    std::shared_ptr<Character> character;
    Move &move;

public:
    ChasePacman(const GameState &g, std::shared_ptr<Character> c, Move &m) : game(g), character(c), move(m) {}

    Status update() override
    {
        // int pos = game.getGhostsPos(ghostIndex);
        // Move dir = (Move)game.getGhostsDir(ghostIndex);
        int pos = character->getPos();
        Move dir = (Move)character->getDirection();

        auto legal = game.getMaze().getGhostLegalMoves(pos, dir);

        int pacPos = game.getPacmanPos();

        auto distances = game.getMaze().distancesFrom(pacPos);

        int bestDist = std::numeric_limits<int>::max();
        Move bestMove = PASS;

        for (auto m : legal)
        {
            int next = game.getMaze().getNeighbour(pos, m);

            if (next >= 0 && next < distances.size() && distances[next] < bestDist)
            {
                bestDist = distances[next];
                bestMove = m;
            }
        }
        if (bestMove == PASS && !legal.empty())
        {
            bestMove = legal[0];
        }

        move = bestMove;
        return BH_SUCCESS;
    }
};

class IsEdible : public Behavior
{
private:
    const GameState &game;
    int ghostIndex;

public:
    IsEdible(const GameState &g, int idx) : game(g), ghostIndex(idx) {}

    Status update() override
    {
        return game.isGhostEdible(ghostIndex) ? BH_SUCCESS : BH_FAILURE;
    }
};

class RandomMove : public Behavior
{
private:
    const GameState &game;
    int ghostIndex;
    // std::shared_ptr<Character> ghost;
    Move &move;

public:
    RandomMove(const GameState &g, int idx, Move &m) : game(g), ghostIndex(idx), move(m) {}

    Status update() override
    {
        int pos = game.getGhostsPos(ghostIndex);
        Move dir = (Move)game.getGhostsDir(ghostIndex);

        auto legal = game.getMaze().getGhostLegalMoves(pos, dir);

        if (!legal.empty())
        {
            move = legal[rand() % legal.size()];
        }
        else
        {
            move = PASS;
        }
        return BH_SUCCESS;
    }
};

class Flee : public Behavior
{
private:
    const GameState &game;
    int ghostIndex;
    // std::shared_ptr<Character> ghost;
    std::shared_ptr<Character> character;
    Move &move;

public:
    Flee(const GameState &g, std::shared_ptr<Character> c, Move &m) : game(g), character(c), move(m) {}

    Status update() override
    {
        // int pos = game.getGhostsPos(ghostIndex);
        // Move dir = (Move)game.getGhostsDir(ghostIndex);
        int pos = character->getPos();
        Move dir = (Move)character->getDirection();

        auto legal = game.getMaze().getGhostLegalMoves(pos, dir);
        std::cout << "Legal size: " << legal.size() << std::endl;

        int pacPos = game.getPacmanPos();

        auto distances = game.getMaze().distancesFrom(pacPos);

        int worstDist = -1;
        Move bestMove = PASS;

        for (auto m : legal)
        {
            int next = game.getMaze().getNeighbour(pos, m);

            if (next >= 0 && next < distances.size() && distances[next] > worstDist)
            {
                worstDist = distances[next];
                bestMove = m;
            }
        }
        if (bestMove == PASS && !legal.empty())
        {
            bestMove = legal[0];
        }

        move = bestMove;
        return BH_SUCCESS;
    }
};

BlinkyController::BlinkyController(std::shared_ptr<Character> character) : Controller(character)
{
}

BlinkyController::~BlinkyController()
{
}
Move BlinkyController::getMove(const GameState &game)
{
    Move move = PASS;
    int ghostIndex = 0;

    auto root = std::make_shared<Selector>();

    auto fleeFilter = std::make_shared<Filter>();
    fleeFilter->addCondition(std::make_shared<IsEdible>(game, ghostIndex));
    fleeFilter->addAction(std::make_shared<Flee>(game, character, move));

    // auto chase = std::make_shared<ChasePacman>(game, ghostIndex, move);
    auto chase = std::make_shared<ChasePacman>(game, character, move);

    root->addChild(fleeFilter);
    root->addChild(chase);
    // auto chase = std::make_shared<Filter>();
    // chase->addCondition(std::make_shared<IsPacmanNear>(game, character));
    // chase->addAction(std::make_shared<ChasePacman>(game, character, move));

    // auto random = std::make_shared<RandomMove>(game, character, move);

    // root->addChild(chase);
    // root->addChild(random);

    root->tick();
    return move;
}
