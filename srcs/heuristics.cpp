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