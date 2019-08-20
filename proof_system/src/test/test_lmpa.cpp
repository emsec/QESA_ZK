#include "test.h"
#include "lmpa/lmpa_simple_zk.h"

using namespace lmpa;

bool test_lmpa()
{
    std::cout << "Testing lmpa..." << std::endl;

    lmpa::Matrix<G> matrix(4, 12);
    for (u32 i = 0; i < matrix.rows(); ++i)
    {
        for (u32 j = 0; j < matrix.cols(); ++j)
        {
            matrix(i, j) = G::rand();
        }
    }

    math::vector<BN> witness;
    for (u32 j = 0; j < matrix.cols(); ++j)
    {
        witness.push_back(BN::rand());
    }

    auto t = matrix * witness;

    simple_zk::ProverContext prover_ctx;
    simple_zk::VerifierContext verifier_ctx;

    simple_zk::begin(prover_ctx, matrix, witness);
    simple_zk::begin(verifier_ctx, matrix, t);
    std::vector<u8> buffer;
    bool continue_prover = true;
    bool continue_verifier = true;
    while (continue_prover || continue_verifier)
    {
        if (continue_prover)
        {
            continue_prover = simple_zk::step_prover(prover_ctx, buffer);
        }
        if (continue_verifier)
        {
            continue_verifier = simple_zk::step_verifier(verifier_ctx, buffer);
        }
    }

    if (simple_zk::get_result(verifier_ctx))
    {
        std::cout << "OK!" << std::endl;
        return true;
    }
    else
    {
        std::cout << "ERROR!" << std::endl;
    }

    return false;
}
