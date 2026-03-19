#include "thistlethwaite.hpp"

int Thistlethwaite::heuristic_phase_1(const Cubie& cube) {
    return _eo_prune[encodeEO(cube)];
}

int Thistlethwaite::heuristic_phase_2(const Cubie& cube) {
    int co = _co_prune[encodeCO(cube)];
    int uds = _uds_prune[encodeUDSlice(cube)];
    if (co < 0 || uds < 0)
        return 0;
    return std::max(co, uds);
}

int Thistlethwaite::heuristic_phase_3(const Cubie& cube) {
    int corner_composite = encodeReducedCP(cube) * 576
        + encodeTetradAPerm(cube) * 24 + encodeTetradBPerm(cube);
    int edge_composite = encodeReducedEP(cube) * 576
        + encodeUDFBPerm(cube) * 24 + encodeUDRLPerm(cube);

    int cp = _phase3_cp_prune[corner_composite];
    int ep = _phase3_ep_prune[edge_composite];

    if (cp < 0 || ep < 0)
        return 0;
    return std::max(cp, ep);
}

int Thistlethwaite::heuristic_phase_4(const Cubie& cube) {
    int cp  = _cp_prune[encodeCP(cube)];
    int ep8 = _ep8_prune[encodeEP8(cube)];
    int ep4 = _ep4_prune[encodeEP4(cube)];

    if (cp < 0 || ep8 < 0 || ep4 < 0)
        return 1000;

    return std::max(cp, std::max(ep8, ep4));
}