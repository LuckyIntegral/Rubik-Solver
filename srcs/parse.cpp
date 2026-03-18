
#include <algorithm>
#include <array>
#include <stdexcept>
#include <sstream>

#include "rubik.hpp"

const char* const kAlgorithmA = "algoA";
const char* const kAlgorithmB = "algoB";
const std::array<const char*, 2> kSupportedAlgorithms = {
    kAlgorithmA,
    kAlgorithmB,
};

constexpr int kArgsMovesOnly = 2;
constexpr int kArgsWithAlgorithm = 4;
constexpr const char* kAlgorithmFlag = "-a";

std::string supported_algorithms_text() {
    std::string text;
    for (std::size_t i = 0; i < kSupportedAlgorithms.size(); ++i) {
        text += kSupportedAlgorithms[i];
        if (i + 1 < kSupportedAlgorithms.size()) {
            text += "|";
        }
    }
    return text;
}

std::string usage(const std::string& binary_name) {
    return "Usage: " + binary_name +
           " <moves> OR " +
           binary_name + " -a <" + supported_algorithms_text() + "> <moves>";
}

bool is_valid_algorithm(const std::string& algorithm) {
    return std::any_of(
        kSupportedAlgorithms.begin(),
        kSupportedAlgorithms.end(),
    [&algorithm](const char* candidate) { return algorithm == candidate; });
}

std::vector<std::string> split_moves(const std::string& moves_str) {
    std::vector<std::string> moves;
    std::istringstream iss(moves_str);
    std::string move;

    while (iss >> move) {
        moves.push_back(move);
    }

    return moves;
}

std::vector<std::string> parse_moves_strict(const std::string& moves_str) {
    const std::vector<std::string> moves = split_moves(moves_str);

    for (const auto& move : moves) {
        const auto pos = std::find(VALID_MOVES.begin(), VALID_MOVES.end(), move);
        if (pos == VALID_MOVES.end()) {
            throw std::invalid_argument("Invalid move: " + move);
        }
    }

    return moves;
}

const SolverCliInput parse_solver_cli_args(int argc, char* argv[]) {
    SolverCliInput input;
    input.algorithm = kAlgorithmA;
    const std::string binary_name = argv[0];

    if (argc == kArgsMovesOnly) {
        input.moves = parse_moves_strict(argv[1]);
        return input;
    }

    if (argc == kArgsWithAlgorithm) {
        if (std::string(argv[1]) != kAlgorithmFlag) {
            throw std::invalid_argument("Expected '-a' as the first argument.\n" +
                                        usage(binary_name));
        }

        input.algorithm = argv[2];
        if (!is_valid_algorithm(input.algorithm)) {
            throw std::invalid_argument("Invalid algorithm: " + input.algorithm +
                                        "\nSupported algorithms: " +
                                        supported_algorithms_text());
        }

        input.moves = parse_moves_strict(argv[3]);
        return input;
    }

    throw std::invalid_argument(usage(binary_name));
}
