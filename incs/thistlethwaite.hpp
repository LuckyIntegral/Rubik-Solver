#ifndef THISTLETHWAITE_HPP
#define THISTLETHWAITE_HPP

#include "cubie.h"

class Thistlethwaite {
    private:
        std::vector<std::string>    _scramble_sequence;
        std::vector<Move>            _path;
        std::vector<int>            _eo_prune;
        std::vector<int>            _co_prune;
        std::vector<int>            _uds_prune;
        Cubie                       _current_cube;
        Cubie                       _solved_cube;
        PhaseRules                  _phase_rules[2];



        void    apply_move(Cubie& cube, Move move);
        Cubie   after_move(Cubie const& cube, Move move);
        void    scramble();
        void    apply_path(Cubie& cube, const std::vector<Move>& path);
        std::string move_to_string(Move move);

        int     encodeEO(const Cubie& cube) const;
        int     encodeCO(const Cubie& cube) const;
        int     encodeUDSlice(const Cubie& cube) const;
        void    init_eo_prune();
        void    init_co_prune();
        void    init_uds_prune();

        int     heuristic_phase_1(const Cubie& cube);
        int     heuristic_phase_2(const Cubie& cube);

        bool    is_phase_1_solved(const Cubie& cube) const;
        bool    is_phase_2_solved(const Cubie& cube) const;
    
        bool    dfs(const Cubie& cube, const PhaseRules& rules, int depth, int limit, std::vector<Move>& path, Move last_move);
        bool    solve_phase(const Cubie& cube, const PhaseRules& rules);

    public:
    Thistlethwaite(std::vector<std::string> scramble_sequence);
    ~Thistlethwaite();
    
    bool    solve(Cubie& cube);
    size_t  get_solution_length() const;

    // testing functions
    bool    is_phase_1_complete(const Cubie& cube) const;
    bool    is_phase_2_complete(const Cubie& cube) const;
    bool    is_pruned();
};

#endif