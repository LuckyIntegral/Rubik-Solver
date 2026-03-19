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
        std::vector<int>            _reduced_cp_prune;
        std::vector<int>            _reduced_ep_prune;
        std::vector<int>            _phase3_cp_prune;
        std::vector<int>            _phase3_ep_prune;
        std::vector<int>            _cp_prune;
        std::vector<int>            _ep8_prune;
        std::vector<int>            _ep4_prune;
        Cubie                       _current_cube;
        Cubie                       _solved_cube;
        PhaseRules                  _phase_rules[4];



        void    apply_move(Cubie& cube, Move move);
        Cubie   after_move(Cubie const& cube, Move move);
        void    scramble();
        void    apply_path(Cubie& cube, const std::vector<Move>& path);
        std::string move_to_string(Move move);

        int     encodeEO(const Cubie& cube) const;
        int     encodeCO(const Cubie& cube) const;
        int     encodeUDSlice(const Cubie& cube) const;
        bool    corners_in_phase3_tetrads(const Cubie& cube) const;
        bool    edges_in_phase3_groups(const Cubie& cube) const;
        bool    has_even_corner_parity(const Cubie& cube) const;
        bool    has_even_edge_parity(const Cubie& cube) const;
        int     encodeReducedCP(const Cubie& cube) const;
        int     encodeReducedEP(const Cubie& cube) const;
        int     encodeTetradAPerm(const Cubie& cube) const;
        int     encodeTetradBPerm(const Cubie& cube) const;
        int     encodeUDFBPerm(const Cubie& cube) const;
        int     encodeUDRLPerm(const Cubie& cube) const;
        int     encodeCP(const Cubie& cube) const;
        int     encodeEP8(const Cubie& cube) const;
        int     encodeEP4(const Cubie& cube) const;
        int     binomial(int n, int k) const;
    
        void    init_eo_prune();
        void    init_co_prune();
        void    init_uds_prune();
        void    init_reduced_cp_prune();
        void    init_reduced_ep_prune();
        void    init_phase3_cp_prune();
        void    init_phase3_ep_prune();
        void    init_cp_prune();
        void    init_ep8_prune();
        void    init_ep4_prune();
        void    init_prune();

        int     heuristic_phase_1(const Cubie& cube);
        int     heuristic_phase_2(const Cubie& cube);
        int     heuristic_phase_3(const Cubie& cube);
        int     heuristic_phase_4(const Cubie& cube);
        
        bool    is_phase_1_solved(const Cubie& cube) const;
        bool    is_phase_2_solved(const Cubie& cube) const;
        bool    is_phase_3_solved(const Cubie& cube) const;
        bool    is_phase_4_solved(const Cubie& cube) const;
        
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
        bool    is_phase_3_complete(const Cubie& cube) const;
        bool    is_phase_4_complete(const Cubie& cube) const;
        bool    is_pruned();
};

#endif