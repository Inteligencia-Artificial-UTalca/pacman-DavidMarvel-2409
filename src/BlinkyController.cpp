#include "BlinkyController.h"
#include "GameState.h"
#include "FSM.h"
#include "Ghost.h"
#include <limits>

class BlinkyChaseState : public FSMState
{
public:
    BlinkyChaseState(std::shared_ptr<Character> c) : FSMState(c)
    {
    }
    Move onUpdate(const GameState &gs) override;
};

class BlinkyScatterState : public FSMState
{
private:
    std::chrono::steady_clock::time_point enterTime;

public:
    BlinkyScatterState(std::shared_ptr<Character> c) : FSMState(c)
    {
    }
    void onEnter(const GameState &) override
    {
        enterTime = std::chrono::steady_clock::now();
    }
    long getElapsedSeconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - enterTime).count();
    }
    Move onUpdate(const GameState &gs) override;
};

class BlinkyFrightenedState : public FSMState
{
public:
    BlinkyFrightenedState(std::shared_ptr<Character> c) : FSMState(c)
    {
    }
    Move onUpdate(const GameState &gs) override;
};

Move BlinkyChaseState::onUpdate(const GameState &gs)
{
    auto pacmanPos = gs.getMaze().getNodePos(gs.getPacmanPos());
    std::vector<Move> moves;
    if (character->getDirection() == PASS)
    {
        moves = gs.getMaze().getPossibleMoves(character->getPos());
    }
    else
    {
        moves = gs.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }
    Move bestMove = PASS;
    float minDist = std::numeric_limits<float>::max();
    for (auto move : moves)
    {
        int nextNode = gs.getMaze().getNeighbour(character->getPos(), move);
        auto nextPos = gs.getMaze().getNodePos(nextNode);
        float dist = euclid2(nextPos, pacmanPos);
        if (dist < minDist)
        {
            minDist = dist;
            bestMove = move;
        }
    }

    return bestMove;
}

Move BlinkyFrightenedState::onUpdate(const GameState &gs)
{
    auto pacmanPos = gs.getMaze().getNodePos(gs.getPacmanPos());
    std::vector<Move> moves;

    if (character->getDirection() == PASS)
    {
        moves = gs.getMaze().getPossibleMoves(character->getPos());
    }
    else
    {
        moves = gs.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }

    Move bestMove = PASS;

    float maxDist = -1;

    for (auto move : moves)
    {
        int nextNode = gs.getMaze().getNeighbour(character->getPos(), move);
        auto nextPos = gs.getMaze().getNodePos(nextNode);
        float dist = euclid2(nextPos, pacmanPos);

        if (dist > maxDist)
        {
            maxDist = dist;
            bestMove = move;
        }
    }

    return bestMove;
}

Move BlinkyScatterState::onUpdate(const GameState &gs)
{
    std::pair<int, int> targetPos = {104, 8};

    std::vector<Move> moves;

    if (character->getDirection() == PASS)
    {
        moves = gs.getMaze().getPossibleMoves(character->getPos());
    }
    else
    {
        moves = gs.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }

    Move bestMove = PASS;
    float minDist = std::numeric_limits<float>::max();

    for (auto move : moves)
    {
        int nextNode = gs.getMaze().getNeighbour(character->getPos(), move);

        auto nextPos = gs.getMaze().getNodePos(nextNode);

        float dist = euclid2(nextPos, targetPos);

        if (dist < minDist)
        {
            minDist = dist;
            bestMove = move;
        }
    }

    return bestMove;
}

class ToFrightenedTransition : public FSMTransition
{
private:
    std::shared_ptr<FSMState> nextState;
    std::shared_ptr<Character> character;

public:
    ToFrightenedTransition(std::shared_ptr<Character> c, std::shared_ptr<FSMState> next) : character(c), nextState(next)
    {
    }
    bool isValid(const GameState &) override
    {
        auto ghost = std::dynamic_pointer_cast<Ghost>(character);
        if (ghost->isEdible())
        {

            return true;
        }
        return false;
    }
    std::shared_ptr<FSMState> getNextState() override
    {
        return nextState;
    }
};

class ExitFrightenedTransition : public FSMTransition
{
private:
    std::shared_ptr<FSMState> *previousState;
    std::shared_ptr<Character> character;

public:
    ExitFrightenedTransition(std::shared_ptr<Character> c, std::shared_ptr<FSMState> *prev) : character(c), previousState(prev)
    {
    }

    bool isValid(const GameState &) override
    {
        auto ghost = std::dynamic_pointer_cast<Ghost>(character);
        return !ghost->isEdible();
    }

    std::shared_ptr<FSMState> getNextState() override
    {
        return *previousState;
    }
};

class ChaseToScatterTransition : public FSMTransition
{
private:
    std::shared_ptr<FSMState> nextState;
    std::chrono::steady_clock::time_point *startTime;

public:
    ChaseToScatterTransition(std::shared_ptr<FSMState> next, std::chrono::steady_clock::time_point *timer) : nextState(next), startTime(timer)
    {
    }

    bool isValid(const GameState &) override
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - *startTime).count();
        return elapsed >= 20;
    }

    std::shared_ptr<FSMState> getNextState() override
    {
        return nextState;
    }
};
class ScatterToChaseTransition : public FSMTransition
{
private:
    std::shared_ptr<BlinkyScatterState> scatterState;
    std::shared_ptr<FSMState> nextState;

public:
    ScatterToChaseTransition(std::shared_ptr<BlinkyScatterState> scatter, std::shared_ptr<FSMState> next) : scatterState(scatter), nextState(next)
    {
    }

    bool isValid(const GameState &) override
    {
        return scatterState->getElapsedSeconds() >= 7;
    }

    std::shared_ptr<FSMState> getNextState() override
    {
        return nextState;
    }
};

class BlinkyFSM : public FiniteStateMachine
{
public:
    std::chrono::steady_clock::time_point startTime;
    std::shared_ptr<FSMState> previousState;
    BlinkyFSM(std::shared_ptr<Character> c);
    Move update(const GameState &gs) override;
    ~BlinkyFSM() {}
};

BlinkyFSM::BlinkyFSM(std::shared_ptr<Character> c) : FiniteStateMachine(c)
{
    startTime = std::chrono::steady_clock::now();
    auto chase = std::make_shared<BlinkyChaseState>(character);
    auto scatter = std::make_shared<BlinkyScatterState>(character);
    auto frightened = std::make_shared<BlinkyFrightenedState>(character);

    chase->addTransition(std::make_shared<ToFrightenedTransition>(character, frightened));
    frightened->addTransition(std::make_shared<ExitFrightenedTransition>(character, &previousState));

    chase->addTransition(std::make_shared<ChaseToScatterTransition>(scatter, &startTime));
    scatter->addTransition(std::make_shared<ToFrightenedTransition>(character, frightened));
    scatter->addTransition(std::make_shared<ScatterToChaseTransition>(scatter, chase));

    initialState = chase;
    activeState = chase;
    previousState = chase;
    states.push_back(chase);
    states.push_back(frightened);
    states.push_back(scatter);
}

Move BlinkyFSM::update(const GameState &gs)
{
    auto t = activeState->getActiveTransition(gs);
    if (t != nullptr)
    {
        activeState->onExit(gs);
        t->onTransition(gs);
        if (dynamic_cast<ToFrightenedTransition *>(t.get()))
        {
            previousState = activeState;
        }
        activeState = t->getNextState();
        startTime = std::chrono::steady_clock::now();
        activeState->onEnter(gs);
    }
    return activeState->onUpdate(gs);
}

BlinkyController::BlinkyController(std::shared_ptr<Character> character) : Controller(character)
{
    fsm = std::make_shared<BlinkyFSM>(character);
}

BlinkyController::~BlinkyController()
{
}
Move BlinkyController::getMove(const GameState &game)
{
    return fsm->update(game);
}
