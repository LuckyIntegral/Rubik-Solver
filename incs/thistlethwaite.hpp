#ifndef THISTLETHWAITE_HPP
#define THISTLETHWAITE_HPP

#include "cubie.h"

class Thistlethwaite {
    private:
        std::vector<std::string>    _scramble_sequence;
        std::vector<std::string>    _path;
        std::vector<int>            _eo_prune;
        std::vector<int>            _co_prune;
        Cubie                       _current_cube;
        Cubie                       _solved_cube;



        void    apply_move(Cubie& cube, Move move);
        Cubie   after_move(Cubie const& cube, Move move);
        void    scramble();
        std::string move_to_string(Move move);

        int     encodeEO(const Cubie& cube);
        int     encodeCO(const Cubie& cube);
        void    init_eo_prune();
        void    init_co_prune();

        bool    is_phase_1_solved(const Cubie& cube) const;
        bool    dfs_phase_1(const Cubie& cube, int depth, int limit, std::vector<std::string>& path, Move last_move);
        bool    solve_phase_1(const Cubie& cube);

    public:
        bool    is_phase_1_complete(const Cubie& cube) const;
        bool    is_pruned();
        Thistlethwaite(std::vector<std::string> scramble_sequence);
        ~Thistlethwaite();

        bool    solve(Cubie& cube);
        size_t  get_solution_length() const;
};

#endif