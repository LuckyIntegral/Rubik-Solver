#include "thistlethwaite.hpp"

static void init_solved_cube(Cubie& cube) {
    for (int i = 0; i < 8; ++i) {
        cube.corner_perm[i] = static_cast<uint8_t>(i);
        cube.corner_ori[i] = 0;
    }
    for (int i = 0; i < 12; ++i) {
        cube.edge_perm[i] = static_cast<uint8_t>(i);
        cube.edge_ori[i] = 0;
    }
}

Thistlethwaite::Thistlethwaite() :
                _eo_prune(2048, -1), _co_prune(2187, -1), _uds_prune(495, -1),
                _phase3_cp_prune(40320, -1), _phase3_ep_prune(40320, -1),
                _cp_prune(40320, -1), _ep8_prune(40320, -1), _ep4_prune(24, -1),
                _current_cube{}, _solved_cube{} {
    init_solved_cube(_current_cube);
    init_solved_cube(_solved_cube);
    static const Move phase_1_moves[] = {
                                            U, U2, U_PRIME,
                                            D, D2, D_PRIME,
                                            L, L2, L_PRIME,
                                            R, R2, R_PRIME,
                                            F, F2, F_PRIME,
                                            B, B2, B_PRIME
                                        };
    static const Move phase_2_moves[] = {
                                            U, U2, U_PRIME,
                                            D, D2, D_PRIME,
                                            L, L2, L_PRIME,
                                            R, R2, R_PRIME,
                                            F2, B2
                                        };
    static const Move phase_3_moves[] = {
                                            U, U2, U_PRIME,
                                            D, D2, D_PRIME,
                                            R2,
                                            L2,
                                            F2,
                                            B2
                                        };
    static const Move phase_4_moves[] = {U2, D2, L2, R2, F2, B2};

    _phase_rules[0] = { .phase = 0, .moves = phase_1_moves,
        .move_count = static_cast<int>(sizeof(phase_1_moves) / sizeof(phase_1_moves[0])) };
    _phase_rules[1] = { .phase = 1, .moves = phase_2_moves,
        .move_count = static_cast<int>(sizeof(phase_2_moves) / sizeof(phase_2_moves[0])) };
    _phase_rules[2] = { .phase = 2, .moves = phase_3_moves,
        .move_count = static_cast<int>(sizeof(phase_3_moves) / sizeof(phase_3_moves[0])) };
    _phase_rules[3] = { .phase = 3, .moves = phase_4_moves,
        .move_count = static_cast<int>(sizeof(phase_4_moves) / sizeof(phase_4_moves[0])) };

    init_prune();
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

void Thistlethwaite::apply_path(Cubie& cube, const std::vector<Move>& path) {
    for (Move m : path) {
        apply_move(cube, m);
    }
}

void Thistlethwaite::reset_current_cube_to_solved() {
    init_solved_cube(_current_cube);
}

void Thistlethwaite::reset_for_next_solve() {
    _path.clear();
    _scramble_sequence.clear();
    init_solved_cube(_current_cube);
}

std::vector<std::string> Thistlethwaite::raw_solution() const {
    std::vector<std::string> out;
    out.reserve(_path.size());
    for (Move m : _path)
        out.push_back(move_to_string(m));
    return out;
}

std::string Thistlethwaite::move_to_string(Move move) const {
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
