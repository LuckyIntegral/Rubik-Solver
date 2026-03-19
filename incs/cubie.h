#ifndef CUBIE_H
#define CUBIE_H

#include <array>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>
#include <queue>

enum Move {
    U, U2, U_PRIME,
    D, D2, D_PRIME,
    L, L2, L_PRIME,
    R, R2, R_PRIME,
    F, F2, F_PRIME,
    B, B2, B_PRIME,
    NOMOVE,
    NUM_FACE_MOVES = 18
};

enum Corner
{
    URF = 0,
    UFL = 1,
    ULB = 2,
    UBR = 3,
    DFR = 4,
    DLF = 5,
    DBL = 6,
    DRB = 7
};

enum Edge
{
    UR = 0,
    UF = 1,
    UL = 2,
    UB = 3,
    DR = 4,
    DF = 5,
    DL = 6,
    DB = 7,
    FR = 8,
    FL = 9,
    BL = 10,
    BR = 11
};

struct Cubie {
    uint8_t corner_perm[8];
    uint8_t corner_ori[8];
    uint8_t edge_perm[12];
    uint8_t edge_ori[12];
};

struct PhaseRules {
    int phase;
    const Move* moves;
    int move_count;
    std::function<bool(const Cubie&)> is_goal;
    std::function<int(const Cubie&)> heuristic;
};

void apply_move(Cubie& cube, Move move);
Cubie after_move(Cubie const& cube, Move move);

#endif