#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "cubie.h"
#include "thistlethwaite.hpp"

static const char* RESET = "\033[0m";
static const char* GREEN = "\033[32m";
static const char* RED = "\033[31m";
static const char* YELLOW = "\033[33m";

static Cubie make_solved_cube() {
    Cubie c{};
    for (int i = 0; i < 8; ++i) { c.corner_perm[i] = i; c.corner_ori[i] = 0; }
    for (int i = 0; i < 12; ++i) { c.edge_perm[i] = i; c.edge_ori[i] = 0; }
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

static bool run_solve(const std::vector<std::string>& scramble, long long* out_ms) {
    Thistlethwaite t(scramble);
    Cubie cube = make_solved_cube();
    apply_scramble(cube, scramble);
    auto start = std::chrono::steady_clock::now();
    bool found = t.solve(cube);
    *out_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    return found;
}

int main() {
    bool all_ok = true;

    // --- Easy scrambles: verify termination ---
    std::vector<std::vector<std::string>> easy = {
        {"F"},
        {"F", "R"},
        {"F", "R", "U"},
        {"R", "U", "R'", "U'"},
        {"F", "R", "U", "R'", "U'", "F'"},
    };

    std::cout << "Easy scrambles:\n";
    for (const auto& s : easy) {
        long long ms;
        bool ok = run_solve(s, &ms);
        std::cout << "  " << s.size() << " moves: " << (ok ? GREEN : RED) << (ok ? "OK" : "FAIL")
                  << RESET << " (" << ms << " ms)\n";
        if (!ok) all_ok = false;
    }
    std::cout << GREEN << "Easy: successful\n" << RESET;

    // --- Hard scrambles: timed, average printed ---
    std::vector<std::vector<std::string>> hard = {
        {"R", "U2", "R'", "U'", "R", "U'", "R'"},
        {"F", "R", "U", "R'", "U'", "F'", "B", "L", "U", "L'", "U'", "B'"},
        {"L", "U", "F", "R'", "D2", "B", "U'", "L'"},
        {"R2", "U", "F2", "D'", "L2", "B", "U2", "R'", "F'"},
        {"F2", "D'", "R2", "B2", "U", "L2", "F'", "D2", "R'", "B", "U2", "L", "F2", "D", "R2", "B'", "U'", "L2", "F", "D2"},
        {"R'", "U2", "F", "L2", "D'", "B2", "R", "U'", "F2", "L", "D2", "B'", "R2", "U", "F'", "L2", "D", "B2", "R'", "U2"},
    };

    std::cout << "\nHard scrambles:\n";
    long long total_ms = 0;
    int n = 0;
    for (const auto& s : hard) {
        long long ms;
        bool ok = run_solve(s, &ms);
        std::cout << "  " << s.size() << " moves: " << (ok ? GREEN : RED) << (ok ? "OK" : "FAIL")
                  << RESET << " " << ms << " ms\n";
        if (!ok) all_ok = false;
        total_ms += ms;
        n++;
    }
    if (n > 0) {
        double avg = static_cast<double>(total_ms) / n;
        std::cout << YELLOW << "\nHard average: " << std::fixed << std::setprecision(1)
                  << avg << " ms (" << n << " scrambles)\n" << RESET;
    }

    std::cout << "\n" << (all_ok ? GREEN : RED) << (all_ok ? "All passed" : "Some failed") << RESET << "\n";
    return all_ok ? 0 : 1;
}
