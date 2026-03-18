#include <chrono>
#include <iostream>
#include <vector>

#include "thistlethwaite.hpp"

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

static void print_header(const std::string& title) {
    std::cout << COLOR_CYAN << "\n=== " << title << " ===" << COLOR_RESET << "\n";
}

static void print_result(const TestResult& tr) {
    const char* color = tr.passed ? COLOR_GREEN : COLOR_RED;
    const char* label = tr.passed ? "PASS" : "FAIL";
    std::cout << "  [" << color << label << COLOR_RESET << "] " << tr.name << "\n";
}

static TestResult test_construct_destruct() {
    try {
        std::vector<std::string> empty;
        {
            Thistlethwaite t(empty);
        }
        return {"1. Construct and destruct (empty scramble)", true};
    } catch (...) {
        return {"1. Construct and destruct (empty scramble)", false};
    }
}

static TestResult test_is_pruned_all_states_reached() {
    std::vector<std::string> empty;
    auto start = std::chrono::steady_clock::now();
    Thistlethwaite t(empty);
    auto end = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "  " << COLOR_YELLOW << "Pruning took " << ms << " ms" << COLOR_RESET << "\n";
    bool ok = t.is_pruned();
    return {"2. is_pruned() - all EO (2048) and CO (2187) states reached", ok};
}

static TestResult test_construct_destruct_with_scramble() {
    try {
        std::vector<std::string> scramble = {"R", "U", "R'"};
        {
            Thistlethwaite t(scramble);
        }
        return {"3. Construct and destruct (with scramble)", true};
    } catch (...) {
        return {"3. Construct and destruct (with scramble)", false};
    }
}

int main() {
    std::vector<TestResult> results;

    print_header("🔧 CONSTRUCT / DESTRUCT TESTS");
    results.push_back(test_construct_destruct());
    results.push_back(test_construct_destruct_with_scramble());

    print_header("📊 PRUNE TABLE TESTS");
    results.push_back(test_is_pruned_all_states_reached());

    int passed = 0;
    for (const auto& r : results) {
        print_result(r);
        if (r.passed) ++passed;
    }

    std::cout << COLOR_YELLOW << "\nSummary: " << passed << " / " << results.size()
              << " tests passed." << COLOR_RESET << "\n";

    return (passed == static_cast<int>(results.size())) ? 0 : 1;
}
