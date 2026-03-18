
#include <map>
#include <vector>

#include "rubik.hpp"

// Testing only, not a real solution
const std::map<std::string, std::string> OPPOSITES = {
    {"R", "R'"},  {"R'", "R"},  {"L", "L'"},  {"L'", "L"},  {"U", "U'"},
    {"U'", "U"},  {"D", "D'"},  {"D'", "D"},  {"F", "F'"},  {"F'", "F"},
    {"B", "B'"},  {"B'", "B"},  {"R2", "R2"}, {"L2", "L2"}, {"U2", "U2"},
    {"D2", "D2"}, {"F2", "F2"}, {"B2", "B2"}};

const std::vector<std::string> solve(const std::vector<std::string>& moves,
                                     const std::string& algorithm) {
    std::vector<std::string> solution;
    if (algorithm == kAlgorithmA) {
        solution.push_back("R");
        solution.push_back("R'");
    }

    for (const auto& move : moves) {
        solution.insert(solution.begin(), OPPOSITES.at(move));
    }

    return solution;
}
