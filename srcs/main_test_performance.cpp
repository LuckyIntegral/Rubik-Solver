#include <iostream>
#include <random>
#include <sstream>
#include <vector>

#include "cubie.h"
#include "thistlethwaite.hpp"

static const char* move_to_str(Move m) {
    static const char* tbl[] = {"U", "U2", "U'", "D", "D2", "D'", "L", "L2", "L'",
                                "R", "R2", "R'", "F", "F2", "F'", "B", "B2", "B'"};
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

int main() {
    static const int SCRAMBLE_LEN_MIN = 20;
    static const int SCRAMBLE_LEN_MAX = 40;

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> len_dist(SCRAMBLE_LEN_MIN, SCRAMBLE_LEN_MAX);
    int len = len_dist(rng);

    std::vector<std::string> scramble = random_scramble(len, rng);

    std::cout << "Scramble (" << scramble.size() << " moves): ";
    std::cout << scramble_to_string(scramble) << "\n\n";

    Thistlethwaite t;
    bool found = t.solve(scramble);

    if (!found) {
        std::cout << "FAILED: solver did not find a solution.\n";
        return 1;
    }

    std::cout << t.performance_solution();

    return 0;
}
