#include "thistlethwaite.hpp"

int Thistlethwaite::heuristic_phase_1(const Cubie& cube) {
    return _eo_prune[encodeEO(cube)];
}

int Thistlethwaite::heuristic_phase_2(const Cubie& cube) {
    return std::max(_co_prune[encodeCO(cube)], _uds_prune[encodeUDSlice(cube)]);
}