
#include <algorithm>
#include <cstdio>

#include "rubik.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <moves>" << std::endl;
        return 1;
    }

    const std::vector<std::string> moves = parse_moves(argv[1]);
    const std::vector<std::string> solution = solve(moves);

    for (const auto& move : solution) {
        std::cout << move << std::endl;
    }

    return 0;
}
