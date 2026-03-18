#include "thistlethwaite.hpp"

static bool is_valid_move(Move move, Move last_move) {
    int move_group = static_cast<int>(move) / 3;
    int last_move_group = static_cast<int>(last_move) / 3;
    return move_group != last_move_group;
}

size_t Thistlethwaite::get_solution_length() const {
    return _path.size();
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

bool Thistlethwaite::dfs(const Cubie& cube, const PhaseRules& rules, int depth, int limit, std::vector<std::string>& path, Move last_move) {
    int h = rules.heuristic(cube);

    if (rules.is_goal(cube))
        return true;

    if (depth + h > limit)
        return false;

    for (int i = 0; i < rules.move_count; ++i) {
        Move move = rules.moves[i];
        if (!is_valid_move(move, last_move))
            continue;
        Cubie next = after_move(cube, move);

        path.push_back(move_to_string(move));

        if (dfs(next, rules, depth + 1, limit, path, move))
            return true;

        path.pop_back();
    }
    return false;
}

bool Thistlethwaite::solve_phase(const Cubie& cube, const PhaseRules& rules) {
    int limit = rules.heuristic(cube);

    while (true) {
        _path.clear();
        if (dfs(cube, rules, 0, limit, _path, NOMOVE))
            return true;
        ++limit;
        if (limit > 50)
            return false;
    }
}