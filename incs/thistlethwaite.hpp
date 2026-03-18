#ifndef THISTLETHWAITE_HPP
#define THISTLETHWAITE_HPP

#include "cubie.h"

class Thistlethwaite {
    private:
        std::vector<std::string>    _scramble_sequence;
        std::vector<int>            _eo_prune;
        std::vector<int>            _co_prune;
        Cubie                       _current_cube;
        Cubie                       _solved_cube;


        void    scramble();
        void    apply_move(Cubie& cube, Move move);
        Cubie   after_move(Cubie const& cube, Move move);
        bool    is_phase_1_solved(const Cubie& cube);
        int     encodeEO(const Cubie& cube);
        int     encodeCO(const Cubie& cube);
        void    init_eo_prune();
        void    init_co_prune();

    public:
        bool    is_pruned();
        Thistlethwaite(std::vector<std::string> scramble_sequence);
        ~Thistlethwaite();

        std::vector<std::string>    solve(Cubie& cube);
};

#endif