#ifndef THISTLETHWAITE_HPP
#define THISTLETHWAITE_HPP

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include "cubie.h"

struct PhaseTelemetry {
    size_t   path_start;
    size_t   path_end;
    int      depth_limit;
    long long ms;
    int      time_units;
    int      space_units;
};

struct SolveTelemetry {
    long long init_ms;
    int      tables_created;
    PhaseTelemetry phases[4];
    long long total_ms;
};

class Thistlethwaite {
    private:
        std::vector<std::string>    _scramble_sequence;
        std::vector<Move>            _path;
        std::vector<int>            _eo_prune;
        std::vector<int>            _co_prune;
        std::vector<int>            _uds_prune;
        std::vector<int>            _phase3_cp_prune;
        std::vector<int>            _phase3_ep_prune;
        std::vector<int>            _cp_prune;
        std::vector<int>            _ep8_prune;
        std::vector<int>            _ep4_prune;
        Cubie                       _current_cube;
        Cubie                       _solved_cube;
        PhaseRules                  _phase_rules[4];
        SolveTelemetry              _telemetry;
        int                         _last_phase_depth;
        int                         _perf_depth_peak;
        std::unordered_map<std::uint64_t, int> _phase4_tt;

        void    apply_move(Cubie& cube, Move move);
        void    reset_current_cube_to_solved();
        void    reset_for_next_solve();
        Cubie   after_move(Cubie const& cube, Move move);
        void    scramble();
        void    apply_path(Cubie& cube, const std::vector<Move>& path);
        std::string move_to_string(Move move) const;

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
        void    init_phase3_cp_prune();
        void    init_phase3_ep_prune();
        void    init_cp_prune();
        void    init_ep8_prune();
        void    init_ep4_prune();
        void    init_prune();

        int     heuristic_phase_1(const Cubie& cube) const;
        int     heuristic_phase_2(const Cubie& cube) const;
        int     heuristic_phase_3(const Cubie& cube) const;
        int     heuristic_phase_4(const Cubie& cube) const;
        bool    phase_is_goal(int phase, const Cubie& cube) const;
        int     phase_heuristic(int phase, const Cubie& cube) const;

        bool    is_phase_1_solved(const Cubie& cube) const;
        bool    is_phase_2_solved(const Cubie& cube) const;
        bool    is_phase_3_solved(const Cubie& cube) const;
        bool    is_phase_4_solved(const Cubie& cube) const;

        std::uint64_t phase4_transposition_key(const Cubie& cube, int limit, int depth) const;
        bool          phase4_transposition_lookup(std::uint64_t key, int& result) const;
        void          phase4_transposition_store(std::uint64_t key, int result);
        void          phase4_transposition_clear();

        int     dfs(Cubie& cube, const PhaseRules& rules, int depth, int limit, std::vector<Move>& path, Move last_move);
        bool    solve_phase(const Cubie& cube, const PhaseRules& rules);

    public:
        Thistlethwaite();
        ~Thistlethwaite();

        bool    solve(const std::vector<std::string>& scramble_moves);
        size_t  get_solution_length() const;
        std::vector<std::string> raw_solution() const;
        std::string human_solution() const;
        std::string performance_solution() const;

        // testing functions
        bool    is_phase_1_complete(const Cubie& cube) const;
        bool    is_phase_2_complete(const Cubie& cube) const;
        bool    is_phase_3_complete(const Cubie& cube) const;
        bool    is_phase_4_complete(const Cubie& cube) const;
};

#endif