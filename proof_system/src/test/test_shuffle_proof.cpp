#include "test.h"
#include "shuffle_proof/shuffle_proof.h"

#include <algorithm>
#include <random>

using namespace shuffle_proof;

bool test_shuffle_proof()
{
    std::cout << "Testing shuffle proof..." << std::endl;
    const u32 shuffle_size = 400;
    auto crs = gen_CRS(shuffle_size);

    auto keys = el_gamal::keygen();

    std::vector<G> plaintexts;
    std::vector<el_gamal::Ciphertext> c_old;
    std::vector<el_gamal::Ciphertext> c_new;
    std::vector<u32> inverse_permutation;
    math::vector<BN> rerandomization;

    for (u32 i = 0; i < shuffle_size; ++i)
    {
        auto ptxt = G::rand();

        auto ctxt = el_gamal::encrypt(keys.pk, ptxt);

        plaintexts.push_back(ptxt);
        c_old.push_back(ctxt);

        inverse_permutation.push_back(i);
    }

    std::shuffle(std::begin(inverse_permutation), std::end(inverse_permutation), std::default_random_engine());

    std::vector<u32> permutation(shuffle_size, (u32)0);
    for (u32 i = 0; i < shuffle_size; ++i)
    {
        auto rho = BN::rand();

        auto ctxt = c_old.at(inverse_permutation.at(i));
        ctxt.c_0 += rho * G::get_gen();
        ctxt.c_1 += rho * keys.pk;

        c_new.push_back(ctxt);
        rerandomization.push_back(rho);

        permutation[inverse_permutation.at(i)] = i;
    }


    std::cout << "Begin protocol..." << std::endl;

    ProverContext prover_ctx(crs);
    VerifierContext verifier_ctx(crs);

    begin(prover_ctx, c_old, c_new, keys.pk, permutation, rerandomization);
    begin(verifier_ctx, c_old, c_new, keys.pk);
    std::vector<u8> buffer;
    bool continue_prover = true;
    bool continue_verifier = true;

    timer::start();
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
    timer::stop();
    std::cout << timer::milliseconds() << "ms" << std::endl;

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
