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
