#include <queue>

#include "thistlethwaite.hpp"

int Thistlethwaite::encodeEO(const Cubie& cube) const {
    int eo = 0;
    for (int i = 0; i < 11; ++i) {
        eo |= (cube.edge_ori[i] << i);
    }
    return eo;
}

int Thistlethwaite::encodeCO(const Cubie& cube) const {
    int co = 0;
    for (int i = 0; i < 7; ++i) {
        co = co * 3 + cube.corner_ori[i];
    }
    return co;
}

static int n_choose_k(int n, int k) {
    if (k < 0 || k > n)
        return 0;

    if (k == 0 || k == n)
        return 1;

    int result = 1;
    for (int i = 1; i <= k; ++i) {
        result = result * (n - k + i) / i;
    }
    return result;
}

static bool is_e_slice_edge(Edge e) {
    return (e == FR || e == FL || e == BR || e == BL);
}

int Thistlethwaite::encodeUDSlice(const Cubie& cube) const {
    int index = 0;
    int r = 4;

    for (int pos = 11; pos >= 0; --pos) {
        if (is_e_slice_edge(static_cast<Edge>(cube.edge_perm[pos]))) {
            index += n_choose_k(pos, r);
            --r;
        }
    }
    return index;
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

        for (int i = 0; i < _phase_rules[0].move_count; ++i) {
            Cubie next = after_move(cube, _phase_rules[0].moves[i]);
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

        for (int i = 0; i < _phase_rules[1].move_count; ++i) {
            Cubie next = after_move(cube, _phase_rules[1].moves[i]);
            int next_co = encodeCO(next);

            if (_co_prune[next_co] == -1) {
                _co_prune[next_co] = current_depth + 1;
                q.push(next);
            }
        }
    }
}

void Thistlethwaite::init_uds_prune() {
    int solved_uds = encodeUDSlice(_solved_cube);
    std::queue<Cubie> q;
    q.push(_solved_cube);
    _uds_prune[solved_uds] = 0;

    while (!q.empty()) {
        Cubie cube = q.front();
        q.pop();

        int current_uds = encodeUDSlice(cube);
        int current_depth = _uds_prune[current_uds];

        for (int i = 0; i < _phase_rules[1].move_count; ++i) {
            Cubie next = after_move(cube, _phase_rules[1].moves[i]);
            int next_uds = encodeUDSlice(next);

            if (_uds_prune[next_uds] == -1) {
                _uds_prune[next_uds] = current_depth + 1;
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