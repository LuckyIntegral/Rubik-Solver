#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

#include "cubie.h"
#include "thistlethwaite.hpp"

static const char* RESET = "\033[0m";
static const char* GREEN = "\033[32m";
static const char* RED = "\033[31m";

static const char* move_to_str(Move m) {
    static const char* tbl[] = {"U", "U2", "U'", "D", "D2", "D'", "L", "L2", "L'", "R", "R2", "R'", "F", "F2", "F'", "B", "B2", "B'"};
    return (m >= U && m <= B_PRIME) ? tbl[static_cast<int>(m)] : "?";
}

static std::vector<std::string> random_scramble(int len, unsigned seed) {
    std::mt19937 rng(seed);
    std::vector<std::string> out;
    int last_group = -1;
    for (int i = 0; i < len; ++i) {
        int group;
        do {
            group = rng() % 6;
        } while (group == last_group);
        last_group = group;
        int variant = rng() % 3;
        Move m = static_cast<Move>(group * 3 + variant);
        out.push_back(move_to_str(m));
    }
    return out;
}

static Cubie make_solved_cube() {
    Cubie c{};
    for (int i = 0; i < 8; ++i) { c.corner_perm[i] = static_cast<uint8_t>(i); c.corner_ori[i] = 0; }
    for (int i = 0; i < 12; ++i) { c.edge_perm[i] = static_cast<uint8_t>(i); c.edge_ori[i] = 0; }
    return c;
}

static Move parse_move(const std::string& m) {
    if (m.empty()) throw std::invalid_argument("Empty move");
    auto p = [&](char c, Move a, Move b, Move d) {
        return m[0] == c ? (m.size() > 1 && m[1] == '\'' ? b : m.size() > 1 && m[1] == '2' ? d : a) : NOMOVE;
    };
    Move r = p('U', U, U_PRIME, U2); if (r != NOMOVE) return r;
    r = p('D', D, D_PRIME, D2); if (r != NOMOVE) return r;
    r = p('L', L, L_PRIME, L2); if (r != NOMOVE) return r;
    r = p('R', R, R_PRIME, R2); if (r != NOMOVE) return r;
    r = p('F', F, F_PRIME, F2); if (r != NOMOVE) return r;
    r = p('B', B, B_PRIME, B2); if (r != NOMOVE) return r;
    throw std::invalid_argument("Invalid move: " + m);
}

static void apply_scramble(Cubie& c, const std::vector<std::string>& s) {
    for (const auto& m : s) apply_move(c, parse_move(m));
}

struct SolveResult {
    bool ok;
    long long ms;
    bool timeout;
    size_t solution_len;
};

static SolveResult run_solve_with_timeout(const std::vector<std::string>& scramble, int timeout_ms) {
    auto task = [&scramble]() {
        Thistlethwaite t(scramble);
        Cubie cube = make_solved_cube();
        apply_scramble(cube, scramble);
        auto start = std::chrono::steady_clock::now();
        bool found = t.solve(cube);
        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        size_t len = found ? t.get_solution_length() : 0;
        return std::make_tuple(found, ms, len);
    };

    auto future = std::async(std::launch::async, task);
    auto status = future.wait_for(std::chrono::milliseconds(timeout_ms));

    if (status == std::future_status::timeout) {
        return {false, timeout_ms, true, 0};
    }
    auto [found, ms, len] = future.get();
    return {found, ms, false, len};
}

int main() {
    bool all_ok = true;

    Thistlethwaite t_init({});
    if (!t_init.is_pruned()) {
        std::cout << RED << "FAIL: is_pruned() - prune tables not fully built\n" << RESET;
        return 1;
    }
    std::cout << GREEN << "is_pruned: OK\n" << RESET;

    std::mt19937 seed_rng(std::random_device{}());
    for (int len = 1; len <= 20; ++len) {
        std::vector<std::string> scramble = random_scramble(len, seed_rng());

        SolveResult r = run_solve_with_timeout(scramble, 2000);

        std::cout << "len " << len << ": "
                  << (r.ok ? GREEN : RED) << (r.timeout ? "TIMEOUT" : (r.ok ? "OK" : "FAIL"))
                  << RESET << " " << r.ms << " ms";
        if (r.ok)
            std::cout << " solution_len " << r.solution_len;
        std::cout << "\n";

        if (!r.ok) all_ok = false;
    }

    std::cout << "\n" << (all_ok ? GREEN : RED) << (all_ok ? "All passed" : "Some failed") << RESET << "\n";
    return all_ok ? 0 : 1;
}
