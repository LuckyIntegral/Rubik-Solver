#include <queue>

#include "thistlethwaite.hpp"

int Thistlethwaite::encodeEO(const Cubie& cube) {
    int eo = 0;
    for (int i = 0; i < 11; ++i) {
        eo |= (cube.edge_ori[i] << i);
    }
    return eo;
}

int Thistlethwaite::encodeCO(const Cubie& cube) {
    int co = 0;
    for (int i = 0; i < 7; ++i) {
        co = co * 3 + cube.corner_ori[i];
    }
    return co;
}

void Thistlethwaite::init_eo_prune() {
    int solved_eo = encodeEO(_solved_cube);

    std::queue<Cubie> q;
    q.push(_solved_cube);
    _eo_prune[solved_eo] = 0;

    while (!q.empty()) {
        Cubie cube = q.front();
        q.pop();

        int current_eo = encodeEO(cube);
        int current_depth = _eo_prune[current_eo];

        for (int i = 0; i < 18; ++i) {
            Cubie next = after_move(cube, static_cast<Move>(i));
            int next_eo = encodeEO(next);

            if (_eo_prune[next_eo] == -1) {
                _eo_prune[next_eo] = current_depth + 1;
                q.push(next);
            }
        }
    }
}

void Thistlethwaite::init_co_prune() {
    int solved_co = encodeCO(_solved_cube);

    std::queue<Cubie> q;
    q.push(_solved_cube);
    _co_prune[solved_co] = 0;

    while (!q.empty()) {
        Cubie cube = q.front();
        q.pop();

        int current_co = encodeCO(cube);
        int current_depth = _co_prune[current_co];

        for (int i = 0; i < 18; ++i) {
            Cubie next = after_move(cube, static_cast<Move>(i));
            int next_co = encodeCO(next);

            if (_co_prune[next_co] == -1) {
                _co_prune[next_co] = current_depth + 1;
                q.push(next);
            }
        }
    }
}

bool Thistlethwaite::is_pruned() {
    for (int i = 0; i < 2048; ++i) {
        if (_eo_prune[i] == -1) {
            return false;
        }
    }
    for (int i = 0; i < 2187; ++i) {
        if (_co_prune[i] == -1) {
            return false;
        }
    }
    return true;
}