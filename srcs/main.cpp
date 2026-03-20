
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

enum class PrintMode { Raw, Human, Performance };

int main(int argc, char* argv[]) {
    try {
        PrintMode mode = PrintMode::Raw;
        std::string moves_arg;

        if (argc != 2 && argc != 3) {
            std::cerr << "Usage: " << argv[0]
                      << " [--human|-h|--performance|-p] \"<moves>\"\n";
            return 1;
        }

        if (argc == 3) {
            const std::string flag(argv[1]);
            if (flag == "--human" || flag == "-h") {
                mode = PrintMode::Human;
            } else if (flag == "--performance" || flag == "-p") {
                mode = PrintMode::Performance;
            } else {
                std::cerr << "Usage: " << argv[0]
                          << " [--human|-h|--performance|-p] \"<moves>\"\n";
                return 1;
            }
        }

        moves_arg = argv[argc - 1];

        const std::vector<std::string> valid_moves = parse_moves(moves_arg);
        if (valid_moves.empty()) {
            std::cerr << "No moves provided\n";
            return 1;
        }

        Thistlethwaite solver;
        solver.solve(valid_moves);

        switch (mode) {
            case PrintMode::Human:
                std::cout << solver.human_solution() << std::endl;
                break;
            case PrintMode::Performance:
                std::cout << solver.performance_solution() << std::endl;
                break;
            case PrintMode::Raw: {
                const std::vector<std::string> solution = solver.raw_solution();
                for (const auto& move : solution) {
                    std::cout << move << std::endl;
                }
                break;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
