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

int Thistlethwaite::binomial(int n, int k) const {
    if (k < 0 || k > n)
        return 0;
    if (k == 0 || k == n)
        return 1;

    int res = 1;
    for (int i = 1; i <= k; ++i)
        res = res * (n - k + i) / i;
    return res;
}

static bool is_e_slice_edge(Edge e) {
    return (e == FR || e == FL || e == BR || e == BL);
}

static bool is_phase3_group_edge(Edge e) {
    return (e == UF || e == UB || e == DF || e == DB);
}

static bool is_udrl_edge(Edge e) {
    return (e == UR || e == UL || e == DR || e == DL);
}

static int lehmer4(const int perm[4]) {
    int index = 0;
    for (int i = 0; i < 4; ++i) {
        int smaller = 0;
        for (int j = i + 1; j < 4; ++j) {
            if (perm[j] < perm[i]) ++smaller;
        }
        int fact = 1;
        for (int k = 1; k <= 3 - i; ++k) fact *= k;
        index += smaller * fact;
    }
    return index;
}

static int corner_to_tetrad_a_id(Corner c) {
    if (c == URF) return 0;
    if (c == ULB) return 1;
    if (c == DLF) return 2;
    if (c == DRB) return 3;
    return -1;
}

static int corner_to_tetrad_b_id(Corner c) {
    if (c == UFL) return 0;
    if (c == UBR) return 1;
    if (c == DFR) return 2;
    if (c == DBL) return 3;
    return -1;
}

static int edge_to_udfb_id(Edge e) {
    if (e == UF) return 0;
    if (e == UB) return 1;
    if (e == DF) return 2;
    if (e == DB) return 3;
    return -1;
}

static int edge_to_udrl_id(Edge e) {
    if (e == UR) return 0;
    if (e == UL) return 1;
    if (e == DR) return 2;
    if (e == DL) return 3;
    return -1;
}

int Thistlethwaite::encodeUDSlice(const Cubie& cube) const {
    int index = 0;
    int r = 4;

    for (int pos = 11; pos >= 0; --pos) {
        if (is_e_slice_edge(static_cast<Edge>(cube.edge_perm[pos]))) {
            index += binomial(pos, r);
            --r;
        }
    }
    return index;
}
// use lehmer code to encode CP, EP
// lehmer: count the number of corners/edges that are smaller than the current corner/edge
static bool is_tetrad_a_corner(Corner c) {
    return (c == URF || c == ULB || c == DLF || c == DRB);
}

bool Thistlethwaite::has_even_corner_parity(const Cubie& cube) const {
    int parity = 0;

    for (int i = 0; i < 8; ++i) {
        for (int j = i + 1; j < 8; ++j) {
            if (cube.corner_perm[i] > cube.corner_perm[j])
                parity ^= 1;
        }
    }
    return parity == 0;
}

bool Thistlethwaite::has_even_edge_parity(const Cubie& cube) const {
    int parity = 0;

    for (int i = 0; i < 12; ++i) {
        for (int j = i + 1; j < 12; ++j) {
            if (cube.edge_perm[i] > cube.edge_perm[j])
                parity ^= 1;
        }
    }
    return parity == 0;
}

int Thistlethwaite::encodeReducedCP(const Cubie& cube) const {
    int index = 0;
    int r = 4;

    for (int pos = 7; pos >= 0; --pos) {
        if (is_tetrad_a_corner(static_cast<Corner>(cube.corner_perm[pos]))) {
            index += binomial(pos, r);
            --r;
        }
        if (r == 0)
            break;
    }
    return index;
}

static bool is_ud_layer_pos(int pos) {
    return (pos == UR || pos == UF || pos == UL || pos == UB ||
            pos == DR || pos == DF || pos == DL || pos == DB);
}

int Thistlethwaite::encodeReducedEP(const Cubie& cube) const {
    int index = 0;
    int r = 4;
    int k = 7;

    for (int pos = 11; pos >= 0; --pos) {
        if (!is_ud_layer_pos(pos))
            continue;

        if (is_phase3_group_edge(static_cast<Edge>(cube.edge_perm[pos]))) {
            index += binomial(k, r);
            --r;
        }
        --k;

        if (r == 0)
            break;
    }
    return index;
}

int Thistlethwaite::encodeTetradAPerm(const Cubie& cube) const {
    int pos_a[4];
    int k = 0;
    for (int pos = 0; pos < 8; ++pos) {
        if (is_tetrad_a_corner(static_cast<Corner>(cube.corner_perm[pos]))) {
            pos_a[k++] = pos;
        }
    }
    int perm[4];
    for (int i = 0; i < 4; ++i) {
        Corner c = static_cast<Corner>(cube.corner_perm[pos_a[i]]);
        perm[i] = corner_to_tetrad_a_id(c);
    }
    return lehmer4(perm);
}

int Thistlethwaite::encodeTetradBPerm(const Cubie& cube) const {
    int pos_b[4];
    int k = 0;
    for (int pos = 0; pos < 8; ++pos) {
        if (!is_tetrad_a_corner(static_cast<Corner>(cube.corner_perm[pos]))) {
            pos_b[k++] = pos;
        }
    }
    int perm[4];
    for (int i = 0; i < 4; ++i) {
        Corner c = static_cast<Corner>(cube.corner_perm[pos_b[i]]);
        perm[i] = corner_to_tetrad_b_id(c);
    }
    return lehmer4(perm);
}

int Thistlethwaite::encodeUDFBPerm(const Cubie& cube) const {
    static const int ud_layer[8] = {UR, UF, UL, UB, DR, DF, DL, DB};
    int pos_udfb[4];
    int k = 0;
    for (int i = 0; i < 8; ++i) {
        int pos = ud_layer[i];
        if (is_phase3_group_edge(static_cast<Edge>(cube.edge_perm[pos]))) {
            pos_udfb[k++] = pos;
        }
    }
    int perm[4];
    for (int i = 0; i < 4; ++i) {
        Edge e = static_cast<Edge>(cube.edge_perm[pos_udfb[i]]);
        perm[i] = edge_to_udfb_id(e);
    }
    return lehmer4(perm);
}

int Thistlethwaite::encodeUDRLPerm(const Cubie& cube) const {
    static const int ud_layer[8] = {UR, UF, UL, UB, DR, DF, DL, DB};
    int pos_udrl[4];
    int k = 0;
    for (int i = 0; i < 8; ++i) {
        int pos = ud_layer[i];
        if (is_udrl_edge(static_cast<Edge>(cube.edge_perm[pos]))) {
            pos_udrl[k++] = pos;
        }
    }
    int perm[4];
    for (int i = 0; i < 4; ++i) {
        Edge e = static_cast<Edge>(cube.edge_perm[pos_udrl[i]]);
        perm[i] = edge_to_udrl_id(e);
    }
    return lehmer4(perm);
}

int Thistlethwaite::encodeCP(const Cubie& cube) const {
    int index = 0;

    for (int i = 0; i < 8; ++i) {
        int smaller = 0;
        for (int j = i + 1; j < 8; ++j) {
            if (cube.corner_perm[j] < cube.corner_perm[i])
                ++smaller;
        }
        index = index * (8 - i) + smaller;
    }
    return index;
}

int Thistlethwaite::encodeEP8(const Cubie& cube) const {
    Edge target[8] = {UR, UF, UL, UB, DR, DF, DL, DB};
    int perm[8];
    int k = 0;

    for (int i = 0; i < 12; ++i) {
        Edge e = static_cast<Edge>(cube.edge_perm[i]);
        for (int j = 0; j < 8; ++j) {
            if (e == target[j]) {
                perm[k++] = j;
                break;
            }
        }
    }

    int index = 0;
    for (int i = 0; i < 8; ++i) {
        int smaller = 0;
        for (int j = i + 1; j < 8; ++j) {
            if (perm[j] < perm[i])
                ++smaller;
        }
        index = index * (8 - i) + smaller;
    }
    return index;
}

int Thistlethwaite::encodeEP4(const Cubie& cube) const {
    Edge target[4] = {FR, FL, BL, BR};
    int perm[4];
    int k = 0;

    for (int i = 0; i < 12; ++i) {
        Edge e = static_cast<Edge>(cube.edge_perm[i]);
        for (int j = 0; j < 4; ++j) {
            if (e == target[j]) {
                perm[k++] = j;
                break;
            }
        }
    }

    int index = 0;
    for (int i = 0; i < 4; ++i) {
        int smaller = 0;
        for (int j = i + 1; j < 4; ++j) {
            if (perm[j] < perm[i])
                ++smaller;
        }
        index = index * (4 - i) + smaller;
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