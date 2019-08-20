#include "test.h"
#include "bulletproofs/range_proof.h"

using namespace bulletproofs;

bool test_bulletproofs()
{
    std::cout << "Testing bulletproofs range proof..." << std::endl;

    u32 range = 60;
    u32 num_commitments = 2;

    COM::CRS com_crs = COM::setup(1);
    bulletproofs::CRS crs = bulletproofs::setup(range, num_commitments, com_crs);

    ProverContext prover_ctx(crs);
    VerifierContext verifier_ctx(crs);

    std::vector<std::tuple<BN, BN>> openings;
    std::vector<G> commitments;
    for (u32 i = 0; i < num_commitments; ++i)
    {
        BN v = BN::rand(range - 1, true);
        BN r = BN::rand();
        openings.push_back({v, r});
        commitments.push_back(COM::commit(com_crs, {v}, r));
    }

    begin(prover_ctx, openings);
    begin(verifier_ctx, commitments);
    std::vector<u8> buffer;
    bool continue_prover = true;
    bool continue_verifier = true;
    while (continue_prover || continue_verifier)
    {
        if (continue_prover)
        {
            continue_prover = step_prover(prover_ctx, buffer);
        }
        if (continue_verifier)
        {
            continue_verifier = step_verifier(verifier_ctx, buffer);
        }
    }

    if (get_result(verifier_ctx))
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
