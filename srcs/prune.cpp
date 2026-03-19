#include <queue>

#include "thistlethwaite.hpp"

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

void Thistlethwaite::init_reduced_cp_prune() {
    int solved_reduced_cp = encodeReducedCP(_solved_cube);
    std::queue<Cubie> q;
    q.push(_solved_cube);
    _reduced_cp_prune[solved_reduced_cp] = 0;

    while (!q.empty()) {
        Cubie cube = q.front();
        q.pop();

        int current_reduced_cp = encodeReducedCP(cube);
        int current_depth = _reduced_cp_prune[current_reduced_cp];

        for (int i = 0; i < _phase_rules[2].move_count; ++i) {
            Cubie next = after_move(cube, _phase_rules[2].moves[i]);
            int next_reduced_cp = encodeReducedCP(next);

            if (_reduced_cp_prune[next_reduced_cp] == -1) {
                _reduced_cp_prune[next_reduced_cp] = current_depth + 1;
                q.push(next);
            }
        }
    }
}

void Thistlethwaite::init_reduced_ep_prune() {
    int solved_reduced_ep = encodeReducedEP(_solved_cube);
    std::queue<Cubie> q;
    q.push(_solved_cube);
    _reduced_ep_prune[solved_reduced_ep] = 0;
    while (!q.empty()) {
        Cubie cube = q.front();
        q.pop();

        int current_reduced_ep = encodeReducedEP(cube);
        int current_depth = _reduced_ep_prune[current_reduced_ep];
        for (int i = 0; i < _phase_rules[2].move_count; ++i) {
            Cubie next = after_move(cube, _phase_rules[2].moves[i]);
            int next_reduced_ep = encodeReducedEP(next);

            if (_reduced_ep_prune[next_reduced_ep] == -1) {
                _reduced_ep_prune[next_reduced_ep] = current_depth + 1;
                q.push(next);
            }
        }
    }
}

void Thistlethwaite::init_phase3_cp_prune() {
    int solved_composite = encodeReducedCP(_solved_cube) * 576
        + encodeTetradAPerm(_solved_cube) * 24 + encodeTetradBPerm(_solved_cube);
    std::queue<Cubie> q;
    q.push(_solved_cube);
    _phase3_cp_prune[solved_composite] = 0;

    while (!q.empty()) {
        Cubie cube = q.front();
        q.pop();

        int current_composite = encodeReducedCP(cube) * 576
            + encodeTetradAPerm(cube) * 24 + encodeTetradBPerm(cube);
        int current_depth = _phase3_cp_prune[current_composite];

        for (int i = 0; i < _phase_rules[2].move_count; ++i) {
            Cubie next = after_move(cube, _phase_rules[2].moves[i]);
            int next_composite = encodeReducedCP(next) * 576
                + encodeTetradAPerm(next) * 24 + encodeTetradBPerm(next);

            if (_phase3_cp_prune[next_composite] == -1) {
                _phase3_cp_prune[next_composite] = current_depth + 1;
                q.push(next);
            }
        }
    }
}

void Thistlethwaite::init_phase3_ep_prune() {
    int solved_composite = encodeReducedEP(_solved_cube) * 576
        + encodeUDFBPerm(_solved_cube) * 24 + encodeUDRLPerm(_solved_cube);
    std::queue<Cubie> q;
    q.push(_solved_cube);
    _phase3_ep_prune[solved_composite] = 0;

    while (!q.empty()) {
        Cubie cube = q.front();
        q.pop();

        int current_composite = encodeReducedEP(cube) * 576
            + encodeUDFBPerm(cube) * 24 + encodeUDRLPerm(cube);
        int current_depth = _phase3_ep_prune[current_composite];

        for (int i = 0; i < _phase_rules[2].move_count; ++i) {
            Cubie next = after_move(cube, _phase_rules[2].moves[i]);
            int next_composite = encodeReducedEP(next) * 576
                + encodeUDFBPerm(next) * 24 + encodeUDRLPerm(next);

            if (_phase3_ep_prune[next_composite] == -1) {
                _phase3_ep_prune[next_composite] = current_depth + 1;
                q.push(next);
            }
        }
    }
}

void Thistlethwaite::init_cp_prune() {
    int solved_cp = encodeCP(_solved_cube);
    std::queue<Cubie> q;
    q.push(_solved_cube);
    _cp_prune[solved_cp] = 0;

    while (!q.empty()) {
        Cubie cube = q.front();
        q.pop();

        int current_cp = encodeCP(cube);
        int current_depth = _cp_prune[current_cp];

        for (int i = 0; i < _phase_rules[3].move_count; ++i) {
            Cubie next = after_move(cube, _phase_rules[3].moves[i]);
            int next_cp = encodeCP(next);
            if (_cp_prune[next_cp] == -1) {
                _cp_prune[next_cp] = current_depth + 1;
                q.push(next);
            }
        }
    }
}

void Thistlethwaite::init_ep8_prune() {
    int solved_ep8 = encodeEP8(_solved_cube);
    std::queue<Cubie> q;
    q.push(_solved_cube);
    _ep8_prune[solved_ep8] = 0;

    while (!q.empty()) {
        Cubie cube = q.front();
        q.pop();

        int current_ep8 = encodeEP8(cube);
        int current_depth = _ep8_prune[current_ep8];
        for (int i = 0; i < _phase_rules[3].move_count; ++i) {
            Cubie next = after_move(cube, _phase_rules[3].moves[i]);
            int next_ep8 = encodeEP8(next);
            if (_ep8_prune[next_ep8] == -1) {
                _ep8_prune[next_ep8] = current_depth + 1;
                q.push(next);
            }
        }
    }
}

void Thistlethwaite::init_ep4_prune() {
    int solved_ep4 = encodeEP4(_solved_cube);
    std::queue<Cubie> q;
    q.push(_solved_cube);
    _ep4_prune[solved_ep4] = 0;

    while (!q.empty()) {
        Cubie cube = q.front();
        q.pop();

        int current_ep4 = encodeEP4(cube);
        int current_depth = _ep4_prune[current_ep4];
        for (int i = 0; i < _phase_rules[3].move_count; ++i) {
            Cubie next = after_move(cube, _phase_rules[3].moves[i]);
            int next_ep4 = encodeEP4(next);
            if (_ep4_prune[next_ep4] == -1) {
                _ep4_prune[next_ep4] = current_depth + 1;
                q.push(next);
            }
        }
    }
}

void Thistlethwaite::init_prune() {
    init_eo_prune();
    init_co_prune();
    init_uds_prune();
    init_reduced_cp_prune();
    init_reduced_ep_prune();
    init_phase3_cp_prune();
    init_phase3_ep_prune();
    init_cp_prune();
    init_ep8_prune();
    init_ep4_prune();
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
    for (int i = 0; i < 495; ++i) {
        if (_uds_prune[i] == -1) {
            return false;
        }
    }
    for (int i = 0; i < 70; ++i) {
        if (_reduced_cp_prune[i] == -1) {
            return false;
        }
    }
    for (int i = 0; i < 70; ++i) {
        if (_reduced_ep_prune[i] == -1) {
            return false;
        }
    }
    for (int i = 0; i < 40320; ++i) {
        if (_phase3_cp_prune[i] == -1) {
            return false;
        }
    }
    for (int i = 0; i < 40320; ++i) {
        if (_phase3_ep_prune[i] == -1) {
            return false;
        }
    }
    for (int i = 0; i < 24; ++i) {
        if (_ep4_prune[i] == -1) {
            return false;
        }
    }
    return true;
}