#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "cubie.h"
#include "thistlethwaite.hpp"

static const char* RESET = "\033[0m";
static const char* BOLD = "\033[1m";
static const char* GREEN = "\033[32m";
static const char* RED = "\033[31m";
static const char* YELLOW = "\033[33m";
static const char* CYAN = "\033[36m";

static const char* move_to_str(Move m) {
    static const char* tbl[] = {"U", "U2", "U'", "D", "D2", "D'", "L", "L2", "L'", "R", "R2", "R'", "F", "F2", "F'", "B", "B2", "B'"};
    return (m >= U && m <= B_PRIME) ? tbl[static_cast<int>(m)] : "?";
}

static std::vector<std::string> random_scramble(int len, std::mt19937& rng) {
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

static std::string scramble_to_string(const std::vector<std::string>& s) {
    std::ostringstream oss;
    for (size_t i = 0; i < s.size(); ++i) {
        if (i > 0) oss << " ";
        oss << s[i];
    }
    return oss.str();
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

static bool test_inverse_moves(std::mt19937& rng) {
    static const Move ALL_MOVES[] = {U, U2, U_PRIME, D, D2, D_PRIME, L, L2, L_PRIME,
                                    R, R2, R_PRIME, F, F2, F_PRIME, B, B2, B_PRIME};
    static const int NUM_RANDOM_STATES = 100;

    for (Move move : ALL_MOVES) {
        for (int i = 0; i < NUM_RANDOM_STATES; ++i) {
            Cubie cube = make_solved_cube();
            if (i > 0)
                apply_scramble(cube, random_scramble(5 + (rng() % 15), rng));

            Cubie test = cube;
            apply_move(test, move);
            apply_move(test, inverse_move(move));

            if (test != cube) {
                std::cout << "BROKEN INVERSE: " << move_to_str(move) << std::endl;
                return false;
            }
        }
    }
    return true;
}

struct SolveResult {
    bool ok;
    long long ms;
    bool timeout;
    size_t solution_len;
};

static SolveResult run_solve_in_process(Thistlethwaite& t, const std::vector<std::string>& scramble) {
    auto start = std::chrono::steady_clock::now();
    bool found = t.solve(scramble);
    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    size_t len = found ? t.get_solution_length() : 0;
    return {found, ms, false, len};
}

struct FailureInfo {
    int test_num;
    std::vector<std::string> scramble;
    std::string reason;
    long long ms;
    size_t solution_len;
};

static std::vector<std::string> parse_scramble_args(int argc, char** argv) {
    if (argc < 2) return {};
    std::string combined;
    for (int i = 1; i < argc; ++i) {
        if (i > 1) combined += " ";
        combined += argv[i];
    }
    std::vector<std::string> out;
    std::istringstream iss(combined);
    std::string move;
    while (iss >> move)
        out.push_back(move);
    return out;
}

static int run_single_scramble(const std::vector<std::string>& scramble) {
    static const int MAX_ACCEPTABLE_MS = 3000;
    static const int MAX_MOVES = 52;

    for (const auto& m : scramble) {
        try {
            parse_move(m);
        } catch (const std::exception& e) {
            std::cout << RED << "Invalid move: " << m << "\n" << RESET;
            return 1;
        }
    }

    Thistlethwaite t;

    std::mt19937 rng_single(std::random_device{}());
    if (!test_inverse_moves(rng_single)) {
        std::cout << RED << "FAIL: inverse_move test - apply+inverse did not restore cube\n" << RESET;
        return 1;
    }

    std::cout << BOLD << "Single scramble test" << RESET << "\n";
    std::cout << "  scramble: " << CYAN << scramble_to_string(scramble) << RESET << "\n";
    auto start = std::chrono::steady_clock::now();
    bool found = t.solve(scramble);
    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    size_t len = found ? t.get_solution_length() : 0;

    std::cout << "  time:  " << ms << " ms\n";
    std::cout << "  moves: " << len << "\n";

    std::vector<std::string> solution = t.raw_solution();
    if (found) {
        std::cout << "  path:  " << CYAN << scramble_to_string(solution) << RESET << "\n";
        std::cout << "\n" << t.human_solution();
    }

    if (!found) {
        std::cout << RED << "  result: FAILED - path not found" << RESET << "\n";
        return 1;
    }

    Cubie verify = make_solved_cube();
    apply_scramble(verify, scramble);
    for (const auto& m : solution)
        apply_move(verify, parse_move(m));
    bool path_correct = t.is_phase_4_complete(verify);
    if (!path_correct) {
        std::cout << RED << "  result: FAILED - wrong path (does not solve cube)" << RESET << "\n";
        return 1;
    }
    if (ms > MAX_ACCEPTABLE_MS) {
        std::cout << RED << "  result: FAILED - timeout (" << ms << " ms > " << MAX_ACCEPTABLE_MS << " ms)" << RESET << "\n";
        return 1;
    }
    if (len > static_cast<size_t>(MAX_MOVES)) {
        std::cout << RED << "  result: FAILED - solution too long (" << len << " > " << MAX_MOVES << " moves)" << RESET << "\n";
        return 1;
    }

    std::cout << GREEN << "  result: OK" << RESET << "\n";
    return 0;
}

int main(int argc, char** argv) {
    std::vector<std::string> arg_scramble = parse_scramble_args(argc, argv);
    if (!arg_scramble.empty()) {
        return run_single_scramble(arg_scramble);
    }

    static const int NUM_TESTS = 5000;
    static const int SCRAMBLE_LEN_MIN = 15;
    static const int SCRAMBLE_LEN_MAX = 40;
    static const int TIMEOUT_MS = 10000;
    static const int MAX_ACCEPTABLE_MS = 3000;
    static const int MAX_MOVES = 52;

    std::mt19937 rng_init(std::random_device{}());
    if (!test_inverse_moves(rng_init)) {
        std::cout << RED << "FAIL: inverse_move test - apply+inverse did not restore cube\n" << RESET;
        return 1;
    }
    std::cout << GREEN << "inverse_moves: OK" << RESET << "\n";
    std::cout << BOLD << "Stress test: " << NUM_TESTS << " tests" << RESET
              << " | scramble len " << SCRAMBLE_LEN_MIN << "-" << SCRAMBLE_LEN_MAX
              << " | fail if > " << MAX_ACCEPTABLE_MS << "ms or > " << MAX_MOVES << " moves\n";

    Thistlethwaite t;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> len_dist(SCRAMBLE_LEN_MIN, SCRAMBLE_LEN_MAX);
    long long total_ms = 0;
    size_t total_turns = 0;
    int passed = 0;
    int count_over_3s = 0;
    int count_timeout = 0;
    int count_solved = 0;
    long long best_ms = -1;
    long long worst_ms = -1;
    size_t best_turns = 0;
    size_t worst_turns = 0;
    std::vector<FailureInfo> failures;

    for (int i = 0; i < NUM_TESTS; ++i) {
        std::vector<std::string> scramble = random_scramble(len_dist(rng), rng);
        SolveResult r = run_solve_in_process(t, scramble);

        if (r.timeout)
            ++count_timeout;

        bool acceptable = true;
        std::string reason;
        if (r.timeout) {
            acceptable = false;
            reason = "timeout (exceeded " + std::to_string(TIMEOUT_MS) + " ms limit)";
        } else if (!r.ok) {
            acceptable = false;
            reason = "solve returned false (path not found)";
        } else if (r.solution_len > static_cast<size_t>(MAX_MOVES)) {
            acceptable = false;
            reason = "solution too long (" + std::to_string(r.solution_len) + " moves > " + std::to_string(MAX_MOVES) + " max)";
        } else if (r.ms > MAX_ACCEPTABLE_MS) {
            acceptable = false;
            reason = "time too slow (" + std::to_string(r.ms) + " ms > " + std::to_string(MAX_ACCEPTABLE_MS) + " ms limit)";
        }

        if (r.ok && !r.timeout) {
            ++count_solved;
            total_ms += r.ms;
            total_turns += r.solution_len;
            if (best_ms < 0 || r.ms < best_ms) best_ms = r.ms;
            if (worst_ms < 0 || r.ms > worst_ms) worst_ms = r.ms;
            if (r.solution_len < best_turns || best_turns == 0) best_turns = r.solution_len;
            if (r.solution_len > worst_turns) worst_turns = r.solution_len;
            if (r.ms > MAX_ACCEPTABLE_MS)
                ++count_over_3s;
        }

        if (acceptable) {
            ++passed;
        } else {
            failures.push_back({i + 1, scramble, reason, r.ms, r.solution_len});
        }

        int done = i + 1;
        int pct = (done * 100) / NUM_TESTS;
        const int bar_width = 32;
        int filled = (done * bar_width) / NUM_TESTS;
        std::cout << "\r  " << CYAN << "[" << RESET;
        for (int j = 0; j < bar_width; ++j) {
            if (j < filled)
                std::cout << GREEN << "\u2588" << RESET;
            else
                std::cout << "\u2591";
        }
        std::cout << CYAN << "] " << RESET << BOLD << done << "/" << NUM_TESTS << RESET << " " << pct << "%" << std::flush;
    }

    std::cout << "\n\n" << BOLD << "Summary" << RESET << "\n";
    std::cout << std::fixed << std::setprecision(2);

    if (count_solved > 0) {
        double avg_ms = static_cast<double>(total_ms) / count_solved;
        double avg_turns = static_cast<double>(total_turns) / count_solved;
        std::cout << "  " << BOLD << "Time (ms):" << RESET << "  best " << GREEN << best_ms << RESET
                  << "  worst " << RED << worst_ms << RESET
                  << "  avg " << avg_ms << "\n";
        std::cout << "  " << BOLD << "Moves:" << RESET << "     best " << GREEN << best_turns << RESET
                  << "  worst " << RED << worst_turns << RESET
                  << "  avg " << avg_turns << "\n";
    }
    std::cout << "  took > 3s: " << YELLOW << count_over_3s << RESET << "\n";
    std::cout << "  timeout 10s: " << YELLOW << count_timeout << RESET << "\n";
    std::cout << "  passed: " << (passed == NUM_TESTS ? GREEN : YELLOW) << passed << "/" << NUM_TESTS << RESET << "\n";

    if (!failures.empty()) {
        std::cout << "\n" << BOLD << RED << "FAIL: " << failures.size() << " test(s) failed" << RESET << "\n\n";
        for (const auto& f : failures) {
            std::cout << RED << "  #" << f.test_num << RESET << " " << f.reason << "\n";
            std::cout << "    " << CYAN << "scramble:" << RESET << " " << scramble_to_string(f.scramble) << "\n\n";
        }
        return 1;
    }

    std::cout << "\n" << GREEN << BOLD << "All " << NUM_TESTS << " stress tests passed" << RESET << "\n";
    return 0;
}
