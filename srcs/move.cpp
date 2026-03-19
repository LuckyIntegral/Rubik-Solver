#include "thistlethwaite.hpp"

// Convension: a -> b -> c -> d -> a
static void cycle4(uint8_t array[], uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    uint8_t tmp = array[a];
    array[a] = array[d];
    array[d] = array[c];
    array[c] = array[b];
    array[b] = tmp;
}

// U and D only changes corners and edges, no orientation changes
static void    move_U(Cubie& cube, int turns = 1) {
    for (int i = 0; i < turns; i++) {
        cycle4(cube.corner_perm, URF, UFL, ULB, UBR);
        cycle4(cube.corner_ori, URF, UFL, ULB, UBR);
        cycle4(cube.edge_perm, UF, UL, UB, UR);
        cycle4(cube.edge_ori, UF, UL, UB, UR);
    }
}

static void    move_D(Cubie& cube, int turns = 1) {
    for (int i = 0; i < turns; i++) {
        cycle4(cube.corner_perm, DFR, DRB, DBL, DLF);
        cycle4(cube.corner_ori, DFR, DRB, DBL, DLF);
        cycle4(cube.edge_perm, DF, DR, DB, DL);
        cycle4(cube.edge_ori, DF, DR, DB, DL);
    }
}

// L and R also twist the 4 corners
static void    move_L(Cubie& cube, int turns = 1) {
    for (int i = 0; i < turns; i++) {
        cycle4(cube.corner_perm, UFL, DLF, DBL, ULB);
        cycle4(cube.corner_ori, UFL, DLF, DBL, ULB);
        cycle4(cube.edge_perm, FL, DL, BL, UL);
        cycle4(cube.edge_ori, FL, DL, BL, UL);
        
        cube.corner_ori[UFL] = (cube.corner_ori[UFL] + 1) % 3;
        cube.corner_ori[ULB] = (cube.corner_ori[ULB] + 2) % 3;
        cube.corner_ori[DBL] = (cube.corner_ori[DBL] + 1) % 3;
        cube.corner_ori[DLF] = (cube.corner_ori[DLF] + 2) % 3;
    }
}

void    move_R(Cubie& cube, int turns = 1) {
    for (int i = 0; i < turns; i++) {
        cycle4(cube.corner_perm, URF, UBR, DRB, DFR);
        cycle4(cube.corner_ori, URF, UBR, DRB, DFR);
        cycle4(cube.edge_perm, FR, UR, BR, DR);
        cycle4(cube.edge_ori, FR, UR, BR, DR);
        
        cube.corner_ori[URF] = (cube.corner_ori[URF] + 2) % 3;
        cube.corner_ori[UBR] = (cube.corner_ori[UBR] + 1) % 3;
        cube.corner_ori[DRB] = (cube.corner_ori[DRB] + 2) % 3;
        cube.corner_ori[DFR] = (cube.corner_ori[DFR] + 1) % 3;
    }
}

// F and B twist the 4 corners also flip edges
static void    move_F(Cubie& cube, int turns = 1) {
    for (int i = 0; i < turns; i++) {
        cycle4(cube.corner_perm, URF, DFR, DLF, UFL);
        cycle4(cube.corner_ori, URF, DFR, DLF, UFL);
        cycle4(cube.edge_perm, FR, DF, FL, UF);
        cycle4(cube.edge_ori, FR, DF, FL, UF);
        
        cube.corner_ori[URF] = (cube.corner_ori[URF] + 1) % 3;
        cube.corner_ori[DFR] = (cube.corner_ori[DFR] + 2) % 3;
        cube.corner_ori[DLF] = (cube.corner_ori[DLF] + 1) % 3;
        cube.corner_ori[UFL] = (cube.corner_ori[UFL] + 2) % 3;
        
        cube.edge_ori[FR] ^= 1;
        cube.edge_ori[DF] ^= 1;
        cube.edge_ori[FL] ^= 1;
        cube.edge_ori[UF] ^= 1;
    }
}

static void    move_B(Cubie& cube, int turns = 1) {
    for (int i = 0; i < turns; i++) {
        cycle4(cube.corner_perm, UBR, ULB, DBL, DRB);
        cycle4(cube.corner_ori, UBR, ULB, DBL, DRB);
        cycle4(cube.edge_perm, BR, UB, BL, DB);
        cycle4(cube.edge_ori, BR, UB, BL, DB);

    cube.corner_ori[UBR] = (cube.corner_ori[UBR] + 2) % 3;
    cube.corner_ori[DRB] = (cube.corner_ori[DRB] + 1) % 3;
    cube.corner_ori[DBL] = (cube.corner_ori[DBL] + 2) % 3;
    cube.corner_ori[ULB] = (cube.corner_ori[ULB] + 1) % 3;

    cube.edge_ori[BR] ^= 1;
    cube.edge_ori[DB] ^= 1;
    cube.edge_ori[BL] ^= 1;
    cube.edge_ori[UB] ^= 1;
    }
}

static void do_apply_move(Cubie& cube, Move move) {
    switch (move) {
        case U:         move_U(cube, 1); break;
        case U2:        move_U(cube, 2); break;
        case U_PRIME:   move_U(cube, 3); break;

        case D:         move_D(cube, 1); break;
        case D2:        move_D(cube, 2); break;
        case D_PRIME:   move_D(cube, 3); break;

        case L:         move_L(cube, 1); break;
        case L2:        move_L(cube, 2); break;
        case L_PRIME:   move_L(cube, 3); break;

        case R:         move_R(cube, 1); break;
        case R2:        move_R(cube, 2); break;
        case R_PRIME:   move_R(cube, 3); break;

        case F:         move_F(cube, 1); break;
        case F2:        move_F(cube, 2); break;
        case F_PRIME:   move_F(cube, 3); break;

        case B:         move_B(cube, 1); break;
        case B2:        move_B(cube, 2); break;
        case B_PRIME:   move_B(cube, 3); break;

        default:
            throw std::invalid_argument("Invalid move");
    }
}

void apply_move(Cubie& cube, Move move) {
    do_apply_move(cube, move);
}

Cubie after_move(Cubie const& cube, Move move) {
    Cubie result = cube;
    do_apply_move(result, move);
    return result;
}

void Thistlethwaite::apply_move(Cubie& cube, Move move) {
    do_apply_move(cube, move);
}

Cubie Thistlethwaite::after_move(Cubie const& cube, Move move) {
    return ::after_move(cube, move);
}