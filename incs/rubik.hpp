
#ifndef RUBIK_HPP
#define RUBIK_HPP

#include <array>
#include <iostream>
#include <string>
#include <vector>

const std::array<std::string, 18> VALID_MOVES = {
    "R",  "L",  "U",  "D",  "F",  "B",  "R'", "L'", "U'",
    "D'", "F'", "B'", "R2", "L2", "U2", "D2", "F2", "B2"};

extern const char* const kAlgorithmA;
extern const char* const kAlgorithmB;
extern const std::array<const char*, 2> kSupportedAlgorithms;

struct SolverCliInput {
    std::vector<std::string> moves;
    std::string algorithm;
};

const SolverCliInput parse_solver_cli_args(int argc, char* argv[]);
const std::vector<std::string> solve(const std::vector<std::string>& moves, const std::string& algorithm);

#endif  // RUBIK_HPP
