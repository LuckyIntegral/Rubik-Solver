#include <chrono>
#include <sstream>

#include "thistlethwaite.hpp"

static const char* const MOVE_STR[] = {
    "U", "U2", "U'", "D", "D2", "D'", "L", "L2", "L'",
    "R", "R2", "R'", "F", "F2", "F'", "B", "B2", "B'"
};

static const char* move_to_str(Move m) {
    return (m >= U && m <= B_PRIME) ? MOVE_STR[static_cast<int>(m)] : "?";
}

static const char* RESET = "\033[0m";
static const char* BOLD = "\033[1m";
static const char* DIM = "\033[2m";
static const char* CYAN = "\033[36m";
static const char* GREEN = "\033[32m";
static const char* YELLOW = "\033[33m";
static const char* MAGENTA = "\033[35m";
static const char* BLUE = "\033[34m";

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
                _current_cube{}, _solved_cube{}, _telemetry{}, _last_phase_depth(0) {
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

    auto t0 = std::chrono::steady_clock::now();
    init_prune();
    auto t1 = std::chrono::steady_clock::now();
    _telemetry.init_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    _telemetry.tables_created = 8;
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
    for (int i = 0; i < 4; ++i) {
        _telemetry.phases[i] = {};
    }
    _telemetry.total_ms = 0;
}

std::vector<std::string> Thistlethwaite::raw_solution() const {
    std::vector<std::string> out;
    out.reserve(_path.size());
    for (Move m : _path)
        out.push_back(move_to_string(m));
    return out;
}

std::string Thistlethwaite::human_solution() const {
    std::ostringstream oss;
    oss << BOLD << MAGENTA << "╔══════════════════════════════════════════╗" << RESET << "\n";
    oss << BOLD << MAGENTA << "║  Rubik Solver (Thistlethwaite Algorithm) ║" << RESET << "\n";
    oss << BOLD << MAGENTA << "╚══════════════════════════════════════════╝" << RESET << "\n\n";

    oss << DIM << "Setup:" << RESET << " Prune tables built in " << CYAN << _telemetry.init_ms
        << RESET << " ms (" << YELLOW << _telemetry.tables_created << RESET
        << " tables for IDA* heuristics)\n\n";

    static const char* const PHASE_DESC[] = {
        "Orient all 12 edges so each can be solved with half-turns only.",
        "Orient corners and move E-slice edges (FR, FL, BR, BL) into the middle layer.",
        "Place corners in tetrads and edges in UD-FB/UD-RL groups for G3.",
        "Solve the cube using only half-turns (U2, D2, L2, R2, F2, B2)."
    };
    static const char* const PHASE_GROUP[] = {
        "G0 → G1",
        "G1 → G2",
        "G2 → G3",
        "G3 → G4"
    };
    static const char* const PHASE_LABEL[] = {
        "Orient edges",
        "Corners + E-slice",
        "Tetrads & groups",
        "Half-turn solve"
    };

    for (int i = 0; i < 4; ++i) {
        const PhaseTelemetry& p = _telemetry.phases[i];
        size_t moves_count = p.path_end - p.path_start;
        oss << BOLD << BLUE << "Phase " << i << " " << RESET << "(" << CYAN << PHASE_GROUP[i]
            << RESET << ") " << YELLOW << PHASE_LABEL[i] << RESET << "\n";
        oss << DIM << "  " << PHASE_DESC[i] << RESET << "\n";
        oss << "  " << DIM << "Time:" << RESET << " " << CYAN << p.ms << RESET << " ms  "
            << DIM << "Depth:" << RESET << " " << YELLOW << p.depth_limit << RESET
            << "  " << DIM << "Moves:" << RESET << " " << GREEN << moves_count << RESET << "\n";
        oss << "  ";
        for (size_t j = p.path_start; j < p.path_end && j < _path.size(); ++j) {
            if (j > p.path_start) oss << " ";
            oss << GREEN << move_to_str(_path[j]) << RESET;
        }
        oss << "\n\n";
    }

    oss << DIM << "──────────────────────────────────────" << RESET << "\n";
    oss << BOLD << "Total: " << CYAN << _telemetry.total_ms << RESET << " ms  "
        << BOLD << "Solution: " << GREEN << _path.size() << RESET << " moves\n";
    oss << DIM << "Full sequence: " << RESET;
    for (size_t i = 0; i < _path.size(); ++i) {
        if (i > 0) oss << " ";
        oss << CYAN << move_to_str(_path[i]) << RESET;
    }
    oss << "\n";
    return oss.str();
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
