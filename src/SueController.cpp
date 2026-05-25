#include "SueController.h"
#include "GameState.h"
#include "FSM.h"
#include "Ghost.h"
#include <limits>

class SueChaseState : public FSMState
{
public:
    SueChaseState(std::shared_ptr<Character> c) : FSMState(c)
    {
    }

    void onEnter(const GameState &) override
    {
        // std::cout << "Sue -> CHASE\n";
    }

    Move onUpdate(const GameState &gs) override;
};

class SueScatterState : public FSMState
{
private:
    std::chrono::steady_clock::time_point enterTime;

public:
    SueScatterState(std::shared_ptr<Character> c) : FSMState(c)
    {
    }
    void onEnter(const GameState &) override
    {
        enterTime = std::chrono::steady_clock::now();
        // std::cout << "Sue -> SCATTER\n";
    }
    long getElapsedSeconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - enterTime).count();
    }
    Move onUpdate(const GameState &gs) override;
};

class SueFrightenedState : public FSMState
{
public:
    SueFrightenedState(std::shared_ptr<Character> c) : FSMState(c)
    {
    }
    void onEnter(const GameState &) override
    {
        // std::cout << "Sue -> FRIGHTENED\n";
    }
    Move onUpdate(const GameState &gs) override;
};

Move SueChaseState::onUpdate(const GameState &gs)
{
    auto pacmanPos = gs.getMaze().getNodePos(gs.getPacmanPos());
    auto myPos = gs.getMaze().getNodePos(character->getPos());
    float distToPacman = euclid2(myPos, pacmanPos);
    std::pair<int, int> target;

    if (distToPacman < 1024)
    {
        // std::cout << "Sue(" << myPos.first << "," << myPos.second << ") " << "Pacman(" << pacmanPos.first << "," << pacmanPos.second << ") " << "Dist=" << distToPacman << std::endl;
        target = {4, 108};
    }
    else
    {
        target = pacmanPos;
        switch (gs.getPacmanDir())
        {
        case UP:
            target.second -= 16;
            break;

        case DOWN:
            target.second += 16;
            break;

        case LEFT:
            target.first -= 16;
            break;

        case RIGHT:
            target.first += 16;
            break;

        default:
            break;
        }
    }

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
        float dist = euclid2(nextPos, target);
        if (dist < minDist)
        {
            minDist = dist;
            bestMove = move;
        }
    }
    return bestMove;
}

Move SueFrightenedState::onUpdate(const GameState &gs)
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

Move SueScatterState::onUpdate(const GameState &gs)
{
    std::pair<int, int> targetPos = {4, 108};

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

class SueToFrightenedTransition : public FSMTransition
{
private:
    std::shared_ptr<FSMState> nextState;
    std::shared_ptr<Character> character;

public:
    SueToFrightenedTransition(std::shared_ptr<Character> c, std::shared_ptr<FSMState> next) : character(c), nextState(next)
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

class SueExitFrightenedTransition : public FSMTransition
{
private:
    std::shared_ptr<FSMState> *previousState;
    std::shared_ptr<Character> character;

public:
    SueExitFrightenedTransition(std::shared_ptr<Character> c, std::shared_ptr<FSMState> *prev) : character(c), previousState(prev)
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

class SueChaseToScatterTransition : public FSMTransition
{
private:
    std::shared_ptr<FSMState> nextState;
    std::chrono::steady_clock::time_point *startTime;

public:
    SueChaseToScatterTransition(std::shared_ptr<FSMState> next, std::chrono::steady_clock::time_point *timer) : nextState(next), startTime(timer)
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

class SueScatterToChaseTransition : public FSMTransition
{
private:
    std::shared_ptr<SueScatterState> scatterState;
    std::shared_ptr<FSMState> nextState;

public:
    SueScatterToChaseTransition(std::shared_ptr<SueScatterState> scatter, std::shared_ptr<FSMState> next) : scatterState(scatter), nextState(next)
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

class SueFSM : public FiniteStateMachine
{
public:
    std::chrono::steady_clock::time_point startTime;
    std::shared_ptr<FSMState> previousState;
    SueFSM(std::shared_ptr<Character> c);
    Move update(const GameState &gs) override;
    ~SueFSM() {}
};

SueFSM::SueFSM(std::shared_ptr<Character> c) : FiniteStateMachine(c)
{
    startTime = std::chrono::steady_clock::now();
    auto chase = std::make_shared<SueChaseState>(character);
    auto scatter = std::make_shared<SueScatterState>(character);
    auto frightened = std::make_shared<SueFrightenedState>(character);

    chase->addTransition(std::make_shared<SueToFrightenedTransition>(character, frightened));
    frightened->addTransition(std::make_shared<SueExitFrightenedTransition>(character, &previousState));

    chase->addTransition(std::make_shared<SueChaseToScatterTransition>(scatter, &startTime));
    scatter->addTransition(std::make_shared<SueToFrightenedTransition>(character, frightened));
    scatter->addTransition(std::make_shared<SueScatterToChaseTransition>(scatter, chase));

    initialState = chase;
    previousState = chase;
    activeState = chase;
    states.push_back(chase);
    states.push_back(frightened);
    states.push_back(scatter);
}

Move SueFSM::update(const GameState &gs)
{
    auto t = activeState->getActiveTransition(gs);
    if (t != nullptr)
    {
        // std::cout << "Cambio de estado detectado\n";
        activeState->onExit(gs);
        t->onTransition(gs);
        if (dynamic_cast<SueToFrightenedTransition *>(t.get()))
        {
            previousState = activeState;
        }
        activeState = t->getNextState();
        startTime = std::chrono::steady_clock::now();
        activeState->onEnter(gs);
    }
    return activeState->onUpdate(gs);
}

SueController::SueController(std::shared_ptr<Character> character) : Controller(character)
{
    fsm = std::make_shared<SueFSM>(character);
}

SueController::~SueController()
{
}

Move SueController::getMove(const GameState &game)
{
    return fsm->update(game);
}
