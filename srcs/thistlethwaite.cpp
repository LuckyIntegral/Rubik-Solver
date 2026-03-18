#include "thistlethwaite.hpp"

Thistlethwaite::Thistlethwaite(std::vector<std::string> scramble_sequence) :
                _scramble_sequence(scramble_sequence), _eo_prune(2048, -1), _co_prune(2187, -1), _current_cube{}, _solved_cube{} {
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
    init_eo_prune();
    init_co_prune();
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

bool Thistlethwaite::is_phase_1_solved(const Cubie& cube) {
    for (int i = 0; i < 12; ++i) {
        if (cube.edge_ori[i] != 0) {
            return false;
        }
    }
    return true;
}
