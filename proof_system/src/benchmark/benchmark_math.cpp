#include "benchmark.h"
#include "math/group.h"

using namespace Group;

bool benchmark_math(u32 iterations)
{
    std::cout << "Benchmarking point multiplications" << std::endl;

    std::vector<BN> scalars;
    scalars.reserve(iterations);
    for (u32 iteration = 0; iteration < iterations; ++iteration)
    {
        scalars.push_back(BN::rand());
    }

    auto point = G::rand();

    timer::start();
    for (u32 iteration = 0; iteration < iterations; ++iteration)
    {
        point *= scalars.at(iteration);
    }
    timer::stop();

    std::cout << "----------------------------" << std::endl;
    std::cout << "average: " << (timer::microseconds() / 1000.0 / static_cast<double>(iterations)) << "ms" << std::endl;

    return true;
}
