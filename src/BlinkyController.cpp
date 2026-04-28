#include "BlinkyController.h"
#include "GameState.h"
#include <limits>


Move getOppositeMove(Move m){
	switch(m){
		case UP: return DOWN;
		case DOWN: return UP;
		case LEFT: return RIGHT;
		case RIGHT: return LEFT;
		default: return PASS;
	}
}

BlinkyController::BlinkyController(std::shared_ptr<Character> character):
	Controller(character){
}

BlinkyController::~BlinkyController() {

}
Move BlinkyController::getMove(const GameState& game){
    int id = 0;
    
    auto ghostPos = game.getGhostsPos(id);
    auto pacmanPos = game.getPacmanPos();

    Move currentDir = (Move)game.getGhostsDir(id);
    
    auto moves = game.getMaze().getGhostLegalMoves(ghostPos,currentDir);

    bool edible = game.isGhostEdible(id);
    Move bestMove = PASS;
    float bestValue = edible ? -1.0f : std::numeric_limits<float>::max();

    if (moves.size() == 1) {
        return moves[0];
    }

    for (Move m : moves){

        if (m == PASS)
            continue;

        int nextPos = game.getMaze().getNeighbour(ghostPos, m);

        if (nextPos == -1)
            continue;

        auto nextCoord = game.getMaze().getNodePos(nextPos);
        auto pacmanCoord = game.getMaze().getNodePos(pacmanPos);

        float dist = euclid2(nextCoord, pacmanCoord);

        if (edible){
            if (dist > bestValue){
                bestValue = dist;
                bestMove = m;
            }
        }
        else{
            if (dist < bestValue){
                bestValue = dist;
                bestMove = m;
            }
        }
    }

    if (bestMove == PASS) {
        for (Move m : moves) {
            if (m != PASS) {
                bestMove = m;
                break;
            }
        }
    }

    return bestMove;
}