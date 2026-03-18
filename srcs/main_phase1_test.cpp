#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "cubie.h"
#include "thistlethwaite.hpp"

// ANSI colors
static const char* COLOR_RESET = "\033[0m";
static const char* COLOR_GREEN = "\033[32m";
static const char* COLOR_RED   = "\033[31m";
static const char* COLOR_CYAN  = "\033[36m";
static const char* COLOR_YELLOW= "\033[33m";

struct TestResult {
    std::string name;
    bool        passed;
};

static void print_header(const std::string& title) {
    std::cout << COLOR_CYAN << "\n=== " << title << " ===" << COLOR_RESET << "\n";
}

static void print_result(const TestResult& tr) {
    const char* color = tr.passed ? COLOR_GREEN : COLOR_RED;
    const char* label = tr.passed ? "PASS" : "FAIL";
    std::cout << "  [" << color << label << COLOR_RESET << "] " << tr.name << "\n";
}

static Cubie make_solved_cube() {
    Cubie cube{};
    for (int i = 0; i < 8; ++i) {
        cube.corner_perm[i] = static_cast<uint8_t>(i);
        cube.corner_ori[i] = 0;
    }
    for (int i = 0; i < 12; ++i) {
        cube.edge_perm[i] = static_cast<uint8_t>(i);
        cube.edge_ori[i] = 0;
    }
    return cube;
}

static Move parse_move(const std::string& move) {
    if (move.empty()) throw std::invalid_argument("Empty move");
    switch (move[0]) {
        case 'U': return (move.size() > 1 && move[1] == '\'') ? U_PRIME : (move.size() > 1 && move[1] == '2') ? U2 : U;
        case 'D': return (move.size() > 1 && move[1] == '\'') ? D_PRIME : (move.size() > 1 && move[1] == '2') ? D2 : D;
        case 'L': return (move.size() > 1 && move[1] == '\'') ? L_PRIME : (move.size() > 1 && move[1] == '2') ? L2 : L;
        case 'R': return (move.size() > 1 && move[1] == '\'') ? R_PRIME : (move.size() > 1 && move[1] == '2') ? R2 : R;
        case 'F': return (move.size() > 1 && move[1] == '\'') ? F_PRIME : (move.size() > 1 && move[1] == '2') ? F2 : F;
        case 'B': return (move.size() > 1 && move[1] == '\'') ? B_PRIME : (move.size() > 1 && move[1] == '2') ? B2 : B;
        default: throw std::invalid_argument("Invalid move: " + move);
    }
}

static void apply_scramble(Cubie& cube, const std::vector<std::string>& scramble) {
    for (const auto& m : scramble) {
        apply_move(cube, parse_move(m));
    }
}

static TestResult test_solved_cube_is_phase1_complete() {
    Cubie cube = make_solved_cube();
    std::vector<std::string> empty;
    Thistlethwaite t(empty);
    bool ok = t.is_phase_1_complete(cube);
    return {"Solved cube is phase 1 complete", ok};
}

static TestResult test_after_F_move_not_phase1_complete() {
    Cubie cube = make_solved_cube();
    apply_move(cube, F);
    std::vector<std::string> empty;
    Thistlethwaite t(empty);
    bool ok = !t.is_phase_1_complete(cube);
    return {"Cube after F move is NOT phase 1 complete", ok};
}

static TestResult test_after_U_move_still_phase1_complete() {
    Cubie cube = make_solved_cube();
    apply_move(cube, U);
    std::vector<std::string> empty;
    Thistlethwaite t(empty);
    bool ok = t.is_phase_1_complete(cube);
    return {"Cube after U move is still phase 1 complete", ok};
}

static TestResult test_solve_phase1_simple_scramble() {
    std::vector<std::string> scramble = {"F", "R", "U"};
    Thistlethwaite t(scramble);
    Cubie cube = make_solved_cube();
    apply_scramble(cube, scramble);
    bool found = t.solve(cube);
    return {"solve() finds solution for F R U", found};
}

static TestResult test_solve_phase1_complex_scrambles() {
    std::vector<std::vector<std::string>> scrambles = {
        {"R", "U", "R'", "U'"},
        {"F", "R", "U", "R'", "U'", "F'"},
        {"R", "U2", "R'", "U'", "R", "U'", "R'"},
        {"F", "R", "U", "R'", "U'", "F'", "B", "L", "U", "L'", "U'", "B'"},
        {"R", "U", "R'", "U", "R", "U2", "R'"},
        {"L", "U", "F", "R'", "D2", "B", "U'", "L'"},
        {"R2", "U", "F2", "D'", "L2", "B", "U2", "R'", "F'"},
        // Hard scrambles (20-move WCA-style)
        {"F2", "D'", "R2", "B2", "U", "L2", "F'", "D2", "R'", "B", "U2", "L", "F2", "D", "R2", "B'", "U'", "L2", "F", "D2"},
        {"R'", "U2", "F", "L2", "D'", "B2", "R", "U'", "F2", "L", "D2", "B'", "R2", "U", "F'", "L2", "D", "B2", "R'", "U2"},
        {"B2", "L", "U'", "R2", "D2", "F'", "B", "L2", "U", "R'", "D2", "F2", "B'", "L'", "U2", "R", "D'", "F", "B2", "L2"},
        {"U2", "R'", "F", "D2", "L2", "B'", "U", "R2", "F'", "D", "L'", "B2", "U'", "R", "F2", "D2", "L", "B", "U2", "R'"},
        {"D", "F2", "R'", "U2", "L", "B'", "D2", "F", "R2", "U'", "L2", "B", "D'", "F2", "R", "U", "L'", "B2", "D2", "F'"},
    };
    long long total_us = 0;
    size_t total_moves = 0;
    for (const auto& scramble : scrambles) {
        Thistlethwaite t(scramble);
        Cubie cube = make_solved_cube();
        apply_scramble(cube, scramble);
        auto start = std::chrono::steady_clock::now();
        bool found = t.solve(cube);
        auto end = std::chrono::steady_clock::now();
        if (!found) {
            return {"solve() finds solution for complex scramble (failed)", false};
        }
        total_us += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        total_moves += t.get_solution_length();
    }
    size_t n = scrambles.size();
    double avg_us = static_cast<double>(total_us) / n;
    double avg_ms = avg_us / 1000.0;
    double avg_moves = static_cast<double>(total_moves) / n;
    std::cout << "  " << COLOR_YELLOW << "Hard scrambles: avg " << std::fixed << std::setprecision(2)
              << avg_ms << " ms (" << static_cast<long long>(avg_us) << " us), avg " << std::setprecision(1)
              << avg_moves << " moves (" << total_moves << " total over " << n << " scrambles)" << COLOR_RESET << "\n";
    return {"solve() finds solution for 12 complex scrambles", true};
}

int main() {
    std::vector<TestResult> results;

    print_header("Phase 1 Completion Tests");
    results.push_back(test_solved_cube_is_phase1_complete());
    results.push_back(test_after_F_move_not_phase1_complete());
    results.push_back(test_after_U_move_still_phase1_complete());
    results.push_back(test_solve_phase1_simple_scramble());
    results.push_back(test_solve_phase1_complex_scrambles());

    int passed = 0;
    for (const auto& r : results) {
        print_result(r);
        if (r.passed) ++passed;
    }

    std::cout << "\nSummary: " << passed << " / " << results.size() << " tests passed.\n";

    return (passed == static_cast<int>(results.size())) ? 0 : 1;
}
