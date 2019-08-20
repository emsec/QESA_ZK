#include "test.h"
#include "benchmark.h"
#include "util.h"
#include <exception>

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    bool result = true;

    if (result)
    {
        result = test_qesa_copy();
        std::cout << "##########################################" << std::endl << std::endl;
    }

    if (result)
    {
        result = test_qesa_range_proof();
        std::cout << "##########################################" << std::endl << std::endl;
    }

    if (result)
    {
        result = test_bulletproofs();
        std::cout << "##########################################" << std::endl << std::endl;
    }

    if (result)
    {
        result = test_lmpa();
        std::cout << "##########################################" << std::endl << std::endl;
    }

    if (result)
    {
        result = test_shuffle_proof();
        std::cout << "##########################################" << std::endl << std::endl;
    }

    if (result)
    {
        result = benchmark_math(10000);
        std::cout << "##########################################" << std::endl
                  << std::endl;
    }

    if (result)
    {
        result = benchmark_arith_circuit();
        std::cout << "##########################################" << std::endl << std::endl;
    }

    if (result)
    {
        result = benchmark_bulletproofs(1);
        std::cout << "##########################################" << std::endl
                  << std::endl;
    }

    if (result)
    {
        result = benchmark_qesa_range_proof(100);
        std::cout << "##########################################" << std::endl
                  << std::endl;
    }

    if (result)
    {
        result = benchmark_shuffle_proof(10);
        std::cout << "##########################################" << std::endl
                  << std::endl;
    }

    return !result;
}
