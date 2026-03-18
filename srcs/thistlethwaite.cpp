#include "thistlethwaite.hpp"

Thistlethwaite::Thistlethwaite(std::vector<std::string> scramble_sequence) :
                _scramble_sequence(scramble_sequence), _eo_prune(2048, -1), _co_prune(2187, -1), _uds_prune(495, -1), _current_cube{}, _solved_cube{} {
    for (int i = 0; i < 8; ++i) {
        _current_cube.corner_perm[i] = static_cast<uint8_t>(i);
        _current_cube.corner_ori[i] = 0;
        _solved_cube.corner_perm[i] = static_cast<uint8_t>(i);
        _solved_cube.corner_ori[i] = 0;
    }
    for (int i = 0; i < 12; ++i) {
        _current_cube.edge_perm[i] = static_cast<uint8_t>(i);
        _current_cube.edge_ori[i] = 0;
        _solved_cube.edge_perm[i] = static_cast<uint8_t>(i);
        _solved_cube.edge_ori[i] = 0;
    }
    static const Move phase_1_moves[] = {U, U2, U_PRIME,
                                        D, D2, D_PRIME,
                                        L, L2, L_PRIME,
                                        R, R2, R_PRIME,
                                        F, F2, F_PRIME,
                                        B, B2, B_PRIME};
    static const Move phase_2_moves[] = {U, U2, U_PRIME,
                                        D, D2, D_PRIME,
                                        L2, R2, F2, B2};

    _phase_rules[0] = {
        .moves = phase_1_moves,
        .move_count = 18,
        .is_goal = [this](const Cubie& c) { return is_phase_1_solved(c); },
        .heuristic = [this](const Cubie& c) { return heuristic_phase_1(c); }
    };
    _phase_rules[1] = {
        .moves = phase_2_moves,
        .move_count = 10,
        .is_goal = [this](const Cubie& c) { return is_phase_2_solved(c); },
        .heuristic = [this](const Cubie& c) { return heuristic_phase_2(c); }
    };

    init_eo_prune();
    init_co_prune();
    init_uds_prune();
    scramble();
}

Thistlethwaite::~Thistlethwaite() {
}

void Thistlethwaite::scramble() {
    for (const auto& move : _scramble_sequence) {
        Move m;
        switch (move[0]) {
            case 'U':
                switch (move.size() > 1 ? move[1] : 0) {
                    case '\'': m = U_PRIME; break;
                    case '2': m = U2; break;
                    default: m = U; break;
                }
                break;
            case 'D':
                switch (move.size() > 1 ? move[1] : 0) {
                    case '\'': m = D_PRIME; break;
                    case '2': m = D2; break;
                    default: m = D; break;
                }
                break;
            case 'L':
                switch (move.size() > 1 ? move[1] : 0) {
                    case '\'': m = L_PRIME; break;
                    case '2': m = L2; break;
                    default: m = L; break;
                }
                break;
            case 'R':
                switch (move.size() > 1 ? move[1] : 0) {
                    case '\'': m = R_PRIME; break;
                    case '2': m = R2; break;
                    default: m = R; break;
                }
                break;
            case 'F':
                switch (move.size() > 1 ? move[1] : 0) {
                    case '\'': m = F_PRIME; break;
                    case '2': m = F2; break;
                    default: m = F; break;
                }
                break;
            case 'B':
                switch (move.size() > 1 ? move[1] : 0) {
                    case '\'': m = B_PRIME; break;
                    case '2': m = B2; break;
                    default: m = B; break;
                }
                break;
            default:
                throw std::invalid_argument("Invalid move: " + move);
        }
        apply_move(_current_cube, m);
    }
}

std::string Thistlethwaite::move_to_string(Move move) {
    switch (move) {
        case U: return "U";
        case U2: return "U2";
        case U_PRIME: return "U'";
        case D: return "D";
        case D2: return "D2";
        case D_PRIME: return "D'";
        case L: return "L";
        case L2: return "L2";
        case L_PRIME: return "L'";
        case R: return "R";
        case R2: return "R2";
        case R_PRIME: return "R'";
        case F: return "F";
        case F2: return "F2";
        case F_PRIME: return "F'";
        case B: return "B";
        case B2: return "B2";
        case B_PRIME: return "B'";
        default: return "?";
    }
}

bool Thistlethwaite::solve(Cubie& cube) {
    if (!solve_phase(cube, _phase_rules[0]))
        return false;
    // return solve_phase(cube, _phase_rules[1]);
    return true;
}