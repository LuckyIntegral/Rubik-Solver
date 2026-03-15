
#ifndef RUBIK_HPP
#define RUBIK_HPP

#include <array>
#include <iostream>
#include <string>
#include <vector>

const std::array<std::string, 18> VALID_MOVES = {
    "R",  "L",  "U",  "D",  "F",  "B",  "R'", "L'", "U'",
    "D'", "F'", "B'", "R2", "L2", "U2", "D2", "F2", "B2"};

const std::vector<std::string> parse_moves(const std::string& moves_str);
const std::vector<std::string> solve(const std::vector<std::string>& moves);

#endif  // RUBIK_HPP
