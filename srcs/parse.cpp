
#include <algorithm>
#include <sstream>

#include "rubik.hpp"

static std::vector<std::string> split_moves(const std::string& moves_str) {
    std::vector<std::string> moves;
    std::istringstream iss(moves_str);
    std::string move;

    while (iss >> move) {
        moves.push_back(move);
    }

    return moves;
}

const std::vector<std::string> parse_moves(const std::string& moves_str) {
    std::vector<std::string> moves = split_moves(moves_str);

    for (const auto& move : moves) {
        const auto pos =
            std::find(VALID_MOVES.begin(), VALID_MOVES.end(), move);
        if (pos == VALID_MOVES.end()) {
            std::cerr << "Invalid move: " << move << std::endl;
            std::exit(1);
        }
    }

    return moves;
}
