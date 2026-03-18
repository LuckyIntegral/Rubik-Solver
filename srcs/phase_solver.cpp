#include "thistlethwaite.hpp"

static bool is_valid_move(Move move, Move last_move, const PhaseRules& rules) {
    int move_group = static_cast<int>(move) / 3;
    int last_move_group = static_cast<int>(last_move) / 3;

    if (rules.move_count == 18) 
        return move_group != last_move_group;
    else if (rules.move_count == 10) {
        if (move > D_PRIME && move != last_move)
            return true;
        else
            return move_group != last_move_group;
    }
    return false;
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

bool Thistlethwaite::is_phase_1_solved(const Cubie& cube) const {
    return encodeEO(cube) == 0;
}

bool Thistlethwaite::is_phase_2_solved(const Cubie& cube) const {
    return encodeCO(cube) == 0 && encodeUDSlice(cube) == encodeUDSlice(_solved_cube);
}

bool Thistlethwaite::dfs(const Cubie& cube, const PhaseRules& rules, int depth, int limit, std::vector<std::string>& path, Move last_move) {
    int h = rules.heuristic(cube);

    if (rules.is_goal(cube))
        return true;

    if (depth + h > limit)
        return false;

    for (int i = 0; i < rules.move_count; ++i) {
        Move move = rules.moves[i];
        if (!is_valid_move(move, last_move, rules))
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