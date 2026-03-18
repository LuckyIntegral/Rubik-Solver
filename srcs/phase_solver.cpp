#include "thistlethwaite.hpp"

static bool is_valid_move(Move move, Move last_move) {
    int move_group = static_cast<int>(move) / 3;
    int last_move_group = static_cast<int>(last_move) / 3;
    return move_group != last_move_group;
}

bool Thistlethwaite::is_phase_1_complete(const Cubie& cube) const {
    return is_phase_1_solved(cube);
}

bool Thistlethwaite::is_phase_1_solved(const Cubie& cube) const {
    for (int i = 0; i < 12; ++i) {
        if (cube.edge_ori[i] != 0) {
            return false;
        }
    }
    return true;
}

bool Thistlethwaite::dfs_phase_1(const Cubie& cube, int depth, int limit, std::vector<std::string>& path, Move last_move) {
    int h = _eo_prune[encodeEO(cube)];

    if (is_phase_1_solved(cube))
        return true;

    if (depth + h > limit)
        return false;

    for (int i = 0; i < 18; ++i) {
        Move move = static_cast<Move>(i);
        if (!is_valid_move(move, last_move))
            continue;
        Cubie next = after_move(cube, move);

        path.push_back(move_to_string(move));

        if (dfs_phase_1(next, depth + 1, limit, path, move))
            return true;

        path.pop_back();
    }
    return false;
}

size_t Thistlethwaite::get_solution_length() const {
    return _path.size();
}

bool Thistlethwaite::solve_phase_1(const Cubie& cube) {
    int limit = _eo_prune[encodeEO(cube)];
    Move    m = NOMOVE;

    while (true) {
        _path.clear();
        if (dfs_phase_1(cube, 0, limit, _path, m))
            return true;
        ++limit;
        if (limit > 50)
            return false;
    }
}