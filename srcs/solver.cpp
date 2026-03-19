#include "thistlethwaite.hpp"

static bool is_valid_move(Move move, Move last_move, const PhaseRules& rules) {
    int move_group = static_cast<int>(move) / 3;
    int last_move_group = static_cast<int>(last_move) / 3;

    if (rules.phase == 0) 
        return move_group != last_move_group;
    else if (rules.phase == 1) {
        if (move > R_PRIME && move != last_move)
            return true;
        else
            return move_group != last_move_group;
    } else if (rules.phase == 2) {
        if (move > D_PRIME && move != last_move)
            return true;
        else
            return move_group != last_move_group;
    }
    return false;
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
        && edges_in_phase3_groups(cube);
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

bool Thistlethwaite::dfs(const Cubie& cube, const PhaseRules& rules, int depth, int limit, std::vector<Move>& path, Move last_move) {
    int h = rules.heuristic(cube);

    if (rules.is_goal(cube))
        return true;

    if (depth + h > limit)
        return false;
    for (int i = 0; i < rules.move_count; ++i) {
        Move move = rules.moves[i];
        if (!is_valid_move(move, last_move, rules)){
            continue;
        }
        Cubie next = after_move(cube, move);

        path.push_back(move);

        if (dfs(next, rules, depth + 1, limit, path, move))
            return true;

        path.pop_back();
    }

    return false;
}

bool Thistlethwaite::solve_phase(const Cubie& cube, const PhaseRules& rules) {
    int limit = rules.heuristic(cube);
    std::vector<Move> path;

    while (true) {
        path.clear();
        if (dfs(cube, rules, 0, limit, path, NOMOVE)) {
            _path.insert(_path.end(), path.begin(), path.end());
            return true;
        }

        ++limit;
        if (limit > 50)
            return false;
    }
}

bool Thistlethwaite::solve(Cubie& cube) {
    for (int i = 0; i < 3; ++i) {

        size_t path_start = _path.size();
        if (!solve_phase(cube, _phase_rules[i]))
            return false;

        for (size_t j = path_start; j < _path.size(); ++j) {
            apply_move(cube, _path[j]);
        }
    }
    return true;
}