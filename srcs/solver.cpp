#include <chrono>
#include <climits>

#include "thistlethwaite.hpp"

static int get_face(Move move) {
    if (move == NOMOVE) return -1;
    return static_cast<int>(move) / 3;
}

static bool is_valid_move(Move move, Move last_move) {
    int move_face = get_face(move);
    int last_face = get_face(last_move);

    if (last_face == -1) return true;  // No previous move

    // Rule 1: Forbid same face twice in a row
    if (move_face == last_face) return false;

    // Rule 2: Opposite faces on same axis - allow only canonical order (smaller face first)
    // Pairs (0,1), (2,3), (4,5) are opposite. Allow only last_face < move_face.
    if ((move_face ^ last_face) == 1)
        return last_face < move_face;

    return true;
}

bool Thistlethwaite::is_phase_1_solved(const Cubie& cube) const {
    return encodeEO(cube) == 0;
}

bool Thistlethwaite::is_phase_2_solved(const Cubie& cube) const {
    return encodeCO(cube) == 0 && encodeUDSlice(cube) == encodeUDSlice(_solved_cube);
}

static bool is_udfb_edge(Edge e) {
    return (e == UF || e == UB || e == DF || e == DB);
}

static bool is_udrl_edge(Edge e) {
    return (e == UR || e == UL || e == DR || e == DL);
}

static bool is_tetrad_a_corner(Corner c) {
    return (c == URF || c == ULB || c == DLF || c == DRB);
}


bool Thistlethwaite::corners_in_phase3_tetrads(const Cubie& cube) const {
    for (int pos = 0; pos < 8; ++pos) {
        Corner piece = static_cast<Corner>(cube.corner_perm[pos]);
        Corner solved_piece = static_cast<Corner>(pos);

        bool solved_pos_in_a = is_tetrad_a_corner(solved_piece);
        bool piece_in_a = is_tetrad_a_corner(piece);

        if (solved_pos_in_a != piece_in_a)
            return false;
    }
    return true;
}

bool Thistlethwaite::edges_in_phase3_groups(const Cubie& cube) const {
    for (int pos = 0; pos < 12; ++pos) {
        Edge piece = static_cast<Edge>(cube.edge_perm[pos]);
        Edge solved_piece = static_cast<Edge>(pos);

        bool solved_pos_udfb = is_udfb_edge(solved_piece);
        bool piece_udfb = is_udfb_edge(piece);

        bool solved_pos_udrl = is_udrl_edge(solved_piece);
        bool piece_udrl = is_udrl_edge(piece);

        if (solved_pos_udfb != piece_udfb)
            return false;
        if (solved_pos_udrl != piece_udrl)
            return false;
    }
    return true;
}

bool Thistlethwaite::is_phase_3_solved(const Cubie& cube) const {
    return is_phase_2_solved(cube)
        && corners_in_phase3_tetrads(cube)
        && edges_in_phase3_groups(cube)
        && has_even_corner_parity(cube)
        && has_even_edge_parity(cube)
        && _cp_prune[encodeCP(cube)] != -1
        && _ep8_prune[encodeEP8(cube)] != -1
        && _ep4_prune[encodeEP4(cube)] != -1;
}

bool Thistlethwaite::is_phase_4_solved(const Cubie& cube) const {
    for (int pos = 0; pos < 8; ++pos) {
        Corner piece = static_cast<Corner>(cube.corner_perm[pos]);
        Corner solved_piece = static_cast<Corner>(pos);

        if (piece != solved_piece || cube.corner_ori[pos] != 0)
            return false;
    }
    for (int pos = 0; pos < 12; ++pos) {
        Edge piece = static_cast<Edge>(cube.edge_perm[pos]);
        Edge solved_piece = static_cast<Edge>(pos);

        if (piece != solved_piece || cube.edge_ori[pos] != 0)
            return false;
    }
    return true;
}

size_t Thistlethwaite::get_solution_length() const {
    return _path.size();
}

bool Thistlethwaite::is_phase_1_complete(const Cubie& cube) const {
    return is_phase_1_solved(cube);
}

bool Thistlethwaite::is_phase_2_complete(const Cubie& cube) const {
    return is_phase_2_solved(cube);
}

bool Thistlethwaite::is_phase_3_complete(const Cubie& cube) const {
    return is_phase_3_solved(cube);
}

bool Thistlethwaite::is_phase_4_complete(const Cubie& cube) const {
    return is_phase_4_solved(cube);
}

bool Thistlethwaite::phase_is_goal(int phase, const Cubie& cube) const {
    switch (phase) {
        case 0: return is_phase_1_solved(cube);
        case 1: return is_phase_2_solved(cube);
        case 2: return is_phase_3_solved(cube);
        case 3: return is_phase_4_solved(cube);
        default: return false;
    }
}

int Thistlethwaite::phase_heuristic(int phase, const Cubie& cube) const {
    switch (phase) {
        case 0: return heuristic_phase_1(cube);
        case 1: return heuristic_phase_2(cube);
        case 2: return heuristic_phase_3(cube);
        case 3: return heuristic_phase_4(cube);
        default: return 0;
    }
}

int Thistlethwaite::dfs(Cubie& cube, const PhaseRules& rules, int depth, int limit, std::vector<Move>& path, Move last_move) {
    if (phase_is_goal(rules.phase, cube))
        return -1;

    int h = phase_heuristic(rules.phase, cube);
    if (depth + h > limit)
        return depth + h;

    int min_exceeded = INT_MAX;
    for (int i = 0; i < rules.move_count; ++i) {
        Move move = rules.moves[i];
        if (!is_valid_move(move, last_move))
            continue;

        apply_move(cube, move);
        path.push_back(move);

        int result = dfs(cube, rules, depth + 1, limit, path, move);
        if (result == -1) {
            apply_move(cube, inverse_move(move));
            return -1;
        }
        if (result > 0 && result < min_exceeded)
            min_exceeded = result;

        path.pop_back();
        apply_move(cube, inverse_move(move));
    }
    return (min_exceeded == INT_MAX) ? 0 : min_exceeded;
}

bool Thistlethwaite::solve_phase(const Cubie& cube, const PhaseRules& rules) {
    int limit = phase_heuristic(rules.phase, cube);
    std::vector<Move> path;
    Cubie work;

    while (true) {
        path.clear();
        work = cube;
        int result = dfs(work, rules, 0, limit, path, NOMOVE);
        if (result == -1) {
            _last_phase_depth = limit;
            _path.insert(_path.end(), path.begin(), path.end());
            return true;
        }
        if (result == 0)
            return false;
        limit = result;
    }
}

bool Thistlethwaite::solve(const std::vector<std::string>& scramble_moves) {
    reset_for_next_solve();
    _scramble_sequence = scramble_moves;
    scramble();

    auto total_start = std::chrono::steady_clock::now();
    for (int i = 0; i < 4; ++i) {
        size_t path_start = _path.size();
        auto phase_start = std::chrono::steady_clock::now();
        if (!solve_phase(_current_cube, _phase_rules[i]))
            return false;
        auto phase_end = std::chrono::steady_clock::now();

        _telemetry.phases[i].path_start = path_start;
        _telemetry.phases[i].path_end = _path.size();
        _telemetry.phases[i].depth_limit = _last_phase_depth;
        _telemetry.phases[i].ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            phase_end - phase_start).count();

        for (size_t j = path_start; j < _path.size(); ++j) {
            apply_move(_current_cube, _path[j]);
        }
    }
    _telemetry.total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - total_start).count();
    return is_phase_4_complete(_current_cube);
}