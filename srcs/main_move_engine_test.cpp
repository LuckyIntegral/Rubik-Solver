#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <string>

#include "cubie.h"

// ANSI colors
static const char* COLOR_RESET = "\033[0m";
static const char* COLOR_GREEN = "\033[32m";
static const char* COLOR_RED   = "\033[31m";
static const char* COLOR_YELLOW= "\033[33m";
static const char* COLOR_CYAN  = "\033[36m";

struct TestResult {
    std::string name;
    bool        passed;
};

static Cubie make_solved() {
    Cubie c;
    for (int i = 0; i < 8; ++i) {
        c.corner_perm[i] = static_cast<uint8_t>(i);
        c.corner_ori[i]  = 0;
    }
    for (int i = 0; i < 12; ++i) {
        c.edge_perm[i] = static_cast<uint8_t>(i);
        c.edge_ori[i]  = 0;
    }
    return c;
}

static bool equal_cubie(const Cubie& a, const Cubie& b) {
    for (int i = 0; i < 8; ++i) {
        if (a.corner_perm[i] != b.corner_perm[i]) return false;
        if (a.corner_ori[i]  != b.corner_ori[i])  return false;
    }
    for (int i = 0; i < 12; ++i) {
        if (a.edge_perm[i] != b.edge_perm[i]) return false;
        if (a.edge_ori[i]  != b.edge_ori[i])  return false;
    }
    return true;
}

static Move inverse_move(Move m) {
    switch (m) {
        case U: return U_PRIME;
        case U_PRIME: return U;
        case U2: return U2;
        case D: return D_PRIME;
        case D_PRIME: return D;
        case D2: return D2;
        case L: return L_PRIME;
        case L_PRIME: return L;
        case L2: return L2;
        case R: return R_PRIME;
        case R_PRIME: return R;
        case R2: return R2;
        case F: return F_PRIME;
        case F_PRIME: return F;
        case F2: return F2;
        case B: return B_PRIME;
        case B_PRIME: return B;
        case B2: return B2;
    }
    // Should never reach here
    return m;
}

static const char* move_name(Move m) {
    switch (m) {
        case U: return "U";
        case U2: return "U2";
        case U_PRIME: return "U'";
        case D: return "D";
        case D2: return "D2";
        case D_PRIME: return "D'";
        case L: return "L";
        case L2: return "L2";
        case L_PRIME: return "L'";
        case R: return "R";
        case R2: return "R2";
        case R_PRIME: return "R'";
        case F: return "F";
        case F2: return "F2";
        case F_PRIME: return "F'";
        case B: return "B";
        case B2: return "B2";
        case B_PRIME: return "B'";
    }
    return "?";
}

// ---------------- BASIC IDENTITY TESTS ----------------

static TestResult test_solved_identity() {
    Cubie c = make_solved();
    bool ok = true;
    for (int i = 0; i < 8; ++i) {
        if (c.corner_perm[i] != i || c.corner_ori[i] != 0) {
            ok = false;
            break;
        }
    }
    for (int i = 0; i < 12; ++i) {
        if (c.edge_perm[i] != i || c.edge_ori[i] != 0) {
            ok = false;
            break;
        }
    }
    return {"1. Solved state (identity)", ok};
}

// ---------------- MOVE INVERSE TESTS ----------------

static TestResult test_move_then_inverse(Move m) {
    Cubie solved = make_solved();
    Cubie c = solved;
    apply_move(c, m);
    apply_move(c, inverse_move(m));
    bool ok = equal_cubie(c, solved);
    return {"2. " + std::string(move_name(m)) + " then " + move_name(inverse_move(m)), ok};
}

static TestResult test_inverse_then_move(Move m) {
    Cubie solved = make_solved();
    Cubie c = solved;
    apply_move(c, inverse_move(m));
    apply_move(c, m);
    bool ok = equal_cubie(c, solved);
    return {"3. " + std::string(move_name(inverse_move(m))) + " then " + move_name(m), ok};
}

// ---------------- POWER TESTS ----------------

static TestResult test_move_four_times(Move m) {
    Cubie solved = make_solved();
    Cubie c = solved;
    for (int i = 0; i < 4; ++i)
        apply_move(c, m);
    bool ok = equal_cubie(c, solved);
    return {"4. " + std::string(move_name(m)) + " x 4 == identity", ok};
}

static TestResult test_double_vs_two_singles(Move single, Move dbl) {
    Cubie solved = make_solved();
    Cubie a = solved;
    Cubie b = solved;
    apply_move(a, dbl);
    apply_move(b, single);
    apply_move(b, single);
    bool ok = equal_cubie(a, b);
    return {"5. " + std::string(move_name(dbl)) + " == " + move_name(single) + " " + move_name(single), ok};
}

static TestResult test_prime_vs_three_singles(Move single, Move prime) {
    Cubie solved = make_solved();
    Cubie a = solved;
    Cubie b = solved;
    apply_move(a, prime);
    for (int i = 0; i < 3; ++i)
        apply_move(b, single);
    bool ok = equal_cubie(a, b);
    return {"6. " + std::string(move_name(prime)) + " == " + move_name(single) + " x 3", ok};
}

// ---------------- PERMUTATION TESTS ----------------

static TestResult test_U_permutation() {
    Cubie c = make_solved();
    apply_move(c, U);

    bool ok = true;
    // Corners: URF -> UFL -> ULB -> UBR -> URF
    ok &= c.corner_perm[URF] == UBR;
    ok &= c.corner_perm[UFL] == URF;
    ok &= c.corner_perm[ULB] == UFL;
    ok &= c.corner_perm[UBR] == ULB;

    // Edges: UF -> UL -> UB -> UR -> UF
    ok &= c.edge_perm[UF] == UR;
    ok &= c.edge_perm[UL] == UF;
    ok &= c.edge_perm[UB] == UL;
    ok &= c.edge_perm[UR] == UB;

    // Orientation unchanged on affected pieces
    ok &= c.corner_ori[URF] == 0;
    ok &= c.corner_ori[UFL] == 0;
    ok &= c.corner_ori[ULB] == 0;
    ok &= c.corner_ori[UBR] == 0;

    ok &= c.edge_ori[UF] == 0;
    ok &= c.edge_ori[UL] == 0;
    ok &= c.edge_ori[UB] == 0;
    ok &= c.edge_ori[UR] == 0;

    return {"7. U permutation + orientation", ok};
}

static TestResult test_D_permutation() {
    Cubie c = make_solved();
    apply_move(c, D);

    bool ok = true;
    // Corners: DFR -> DRB -> DBL -> DLF -> DFR
    ok &= c.corner_perm[DFR] == DLF;
    ok &= c.corner_perm[DRB] == DFR;
    ok &= c.corner_perm[DBL] == DRB;
    ok &= c.corner_perm[DLF] == DBL;

    // Edges: DF -> DR -> DB -> DL -> DF
    ok &= c.edge_perm[DF] == DL;
    ok &= c.edge_perm[DR] == DF;
    ok &= c.edge_perm[DB] == DR;
    ok &= c.edge_perm[DL] == DB;

    // Orientation unchanged on affected pieces
    ok &= c.corner_ori[DFR] == 0;
    ok &= c.corner_ori[DRB] == 0;
    ok &= c.corner_ori[DBL] == 0;
    ok &= c.corner_ori[DLF] == 0;

    ok &= c.edge_ori[DF] == 0;
    ok &= c.edge_ori[DR] == 0;
    ok &= c.edge_ori[DB] == 0;
    ok &= c.edge_ori[DL] == 0;

    return {"8. D permutation + orientation", ok};
}

// ---------------- ORIENTATION TESTS ----------------

static TestResult test_F_edge_flip_and_corner_twist() {
    Cubie c = make_solved();
    apply_move(c, F);

    bool ok = true;

    // Edge flips: FR, DF, FL, UF toggled
    ok &= c.edge_ori[FR] == 1;
    ok &= c.edge_ori[DF] == 1;
    ok &= c.edge_ori[FL] == 1;
    ok &= c.edge_ori[UF] == 1;
    for (int i = 0; i < 12; ++i) {
        if (i == FR || i == DF || i == FL || i == UF) continue;
        ok &= c.edge_ori[i] == 0;
    }

    // Corner twists for F:
    // URF +1, DFR +2, DLF +1, UFL +2
    ok &= c.corner_ori[URF] == 1;
    ok &= c.corner_ori[DFR] == 2;
    ok &= c.corner_ori[DLF] == 1;
    ok &= c.corner_ori[UFL] == 2;

    return {"9 & 13. F edge flips + corner twists", ok};
}

static TestResult test_B_edge_flip_and_corner_twist() {
    Cubie c = make_solved();
    apply_move(c, B);

    bool ok = true;

    // Edge flips: BR, DB, BL, UB toggled
    ok &= c.edge_ori[BR] == 1;
    ok &= c.edge_ori[DB] == 1;
    ok &= c.edge_ori[BL] == 1;
    ok &= c.edge_ori[UB] == 1;
    for (int i = 0; i < 12; ++i) {
        if (i == BR || i == DB || i == BL || i == UB) continue;
        ok &= c.edge_ori[i] == 0;
    }

    // Corner twists for B:
    // UBR +2, DRB +1, DBL +2, ULB +1
    ok &= c.corner_ori[UBR] == 2;
    ok &= c.corner_ori[DRB] == 1;
    ok &= c.corner_ori[DBL] == 2;
    ok &= c.corner_ori[ULB] == 1;

    return {"10 & 16. B edge flips + corner twists", ok};
}

static TestResult test_R_no_edge_flip_and_corner_twist() {
    Cubie c = make_solved();
    apply_move(c, R);

    bool ok = true;

    // No edge flips for R
    for (int i = 0; i < 12; ++i)
        ok &= c.edge_ori[i] == 0;

    // Corner twists for R:
    // URF +2, UBR +1, DRB +2, DFR +1
    ok &= c.corner_ori[URF] == 2;
    ok &= c.corner_ori[UBR] == 1;
    ok &= c.corner_ori[DRB] == 2;
    ok &= c.corner_ori[DFR] == 1;

    return {"11 & 14. R corner twists, no edge flips", ok};
}

static TestResult test_L_no_edge_flip_and_corner_twist() {
    Cubie c = make_solved();
    apply_move(c, L);

    bool ok = true;

    // No edge flips for L
    for (int i = 0; i < 12; ++i)
        ok &= c.edge_ori[i] == 0;

    // Corner twists for L:
    // UFL +1, ULB +2, DBL +1, DLF +2
    ok &= c.corner_ori[UFL] == 1;
    ok &= c.corner_ori[ULB] == 2;
    ok &= c.corner_ori[DBL] == 1;
    ok &= c.corner_ori[DLF] == 2;

    return {"12 & 15. L corner twists, no edge flips", ok};
}

// ---------------- INVARIANT TESTS ----------------

static int corner_ori_sum(const Cubie& c) {
    int s = 0;
    for (int i = 0; i < 8; ++i)
        s += c.corner_ori[i];
    return s;
}

static int edge_ori_sum(const Cubie& c) {
    int s = 0;
    for (int i = 0; i < 12; ++i)
        s += c.edge_ori[i];
    return s;
}

static TestResult test_orientation_invariants() {
    Cubie c = make_solved();

    // Apply some sequence
    std::vector<Move> seq = {U, R, F, D_PRIME, L2, B};
    for (Move m : seq)
        apply_move(c, m);

    bool ok = true;
    ok &= (corner_ori_sum(c) % 3) == 0;
    ok &= (edge_ori_sum(c)   % 2) == 0;

    return {"17 & 18. Orientation invariants (corner %3, edge %2)", ok};
}

// ---------------- RANDOM SEQUENCE TEST ----------------

static TestResult test_random_sequence_inverse() {
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> dist(0, 17); // 18 moves

    for (int trial = 0; trial < 10; ++trial) {
        Cubie start = make_solved();
        Cubie c = start;

        std::vector<Move> seq;
        int len = 20;
        for (int i = 0; i < len; ++i) {
            Move m = static_cast<Move>(dist(rng));
            seq.push_back(m);
            apply_move(c, m);
        }
        // Apply inverse in reverse order
        for (int i = len - 1; i >= 0; --i) {
            apply_move(c, inverse_move(seq[i]));
        }
        if (!equal_cubie(c, start)) {
            return {"19. Random sequence + inverse", false};
        }
    }
    return {"19. Random sequence + inverse", true};
}

// ---------------- RUNNER ----------------

static void print_header(const std::string& title) {
    std::cout << COLOR_CYAN << "\n=== " << title << " ===" << COLOR_RESET << "\n";
}

static void print_result(const TestResult& tr) {
    const char* color = tr.passed ? COLOR_GREEN : COLOR_RED;
    const char* label = tr.passed ? "PASS" : "FAIL";
    std::cout << "  [" << color << label << COLOR_RESET << "] " << tr.name << "\n";
}

int main() {
    std::vector<TestResult> results;

    print_header("🧪 BASIC IDENTITY TESTS");
    results.push_back(test_solved_identity());

    print_header("🔁 MOVE INVERSE TESTS");
    Move basic_moves[] = {U, D, L, R, F, B};
    for (Move m : basic_moves) {
        results.push_back(test_move_then_inverse(m));
        results.push_back(test_inverse_then_move(m));
    }

    print_header("🔄 POWER TESTS");
    for (Move m : basic_moves) {
        results.push_back(test_move_four_times(m));
    }
    results.push_back(test_double_vs_two_singles(U, U2));
    results.push_back(test_double_vs_two_singles(D, D2));
    results.push_back(test_double_vs_two_singles(L, L2));
    results.push_back(test_double_vs_two_singles(R, R2));
    results.push_back(test_double_vs_two_singles(F, F2));
    results.push_back(test_double_vs_two_singles(B, B2));

    results.push_back(test_prime_vs_three_singles(U, U_PRIME));
    results.push_back(test_prime_vs_three_singles(D, D_PRIME));
    results.push_back(test_prime_vs_three_singles(L, L_PRIME));
    results.push_back(test_prime_vs_three_singles(R, R_PRIME));
    results.push_back(test_prime_vs_three_singles(F, F_PRIME));
    results.push_back(test_prime_vs_three_singles(B, B_PRIME));

    print_header("🔀 PERMUTATION TESTS");
    results.push_back(test_U_permutation());
    results.push_back(test_D_permutation());

    print_header("🔁 ORIENTATION TESTS");
    results.push_back(test_F_edge_flip_and_corner_twist());
    results.push_back(test_B_edge_flip_and_corner_twist());
    results.push_back(test_R_no_edge_flip_and_corner_twist());
    results.push_back(test_L_no_edge_flip_and_corner_twist());

    print_header("🧮 INVARIANT TESTS");
    results.push_back(test_orientation_invariants());

    print_header("🎲 RANDOM SEQUENCE TEST");
    results.push_back(test_random_sequence_inverse());

    int passed = 0;
    for (const auto& r : results) {
        print_result(r);
        if (r.passed) ++passed;
    }

    std::cout << COLOR_YELLOW << "\nSummary: " << passed << " / " << results.size()
              << " tests passed." << COLOR_RESET << "\n";

    return (passed == static_cast<int>(results.size())) ? 0 : 1;
}

