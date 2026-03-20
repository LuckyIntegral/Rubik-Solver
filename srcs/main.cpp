
#include <exception>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>

#include "thistlethwaite.hpp"

const std::array<std::string, 18> VALID_MOVES = {
    "R",  "L",  "U",  "D",  "F",  "B",  "R'", "L'", "U'",
    "D'", "F'", "B'", "R2", "L2", "U2", "D2", "F2", "B2"};

static const std::vector<std::string> parse_moves(const std::string& moves_str) {
    std::vector<std::string> moves;
    std::istringstream iss(moves_str);
    std::string move;

    while (iss >> move) {
        const auto pos =
            std::find(VALID_MOVES.begin(), VALID_MOVES.end(), move);
        if (pos == VALID_MOVES.end()) {
            std::cerr << "Invalid move: " << move << std::endl;
            std::exit(1);
        }
        moves.push_back(move);
    }

    return moves;
}

int main(int argc, char* argv[]) {
    try {
        bool print_human = false;
        std::string moves_arg;

        if (argc != 2 && argc != 3) {
            std::cerr << "Usage: " << argv[0] << " [--human|-p] \"<moves>\"\n";
            return 1;
        }

        if (argc == 3) {
            if (std::string(argv[1]) != "--human" && std::string(argv[1]) != "-p") {
                std::cerr << "Usage: " << argv[0] << " [--human|-p] \"<moves>\"\n";
                return 1;
            }
            print_human = true;
        }

        moves_arg = argv[argc - 1];

        const std::vector<std::string> valid_moves = parse_moves(moves_arg);
        if (valid_moves.empty()) {
            std::cerr << "No moves provided\n";
            return 1;
        }

        Thistlethwaite solver;
        solver.solve(valid_moves);

        if (print_human) {
            std::cout << solver.human_solution() << std::endl;
        } else {
            const std::vector<std::string> solution = solver.raw_solution();

            for (const auto& move : solution) {
                std::cout << move << std::endl;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
