
#include <exception>

#include "rubik.hpp"

int main(int argc, char* argv[]) {
    try {
        const SolverCliInput input = parse_solver_cli_args(argc, argv);
        const std::vector<std::string> solution =
            solve(input.moves, input.algorithm);

        for (const auto& move : solution) {
            std::cout << move << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
