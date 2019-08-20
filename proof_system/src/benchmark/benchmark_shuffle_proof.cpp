#include "benchmark.h"
#include "shuffle_proof/shuffle_proof.h"

#include <algorithm>
#include <random>

using namespace Group;

static bool benchmark(u32 iterations, u32 shuffle_size)
{
    std::cout << "Benchmarking shuffle proof of size " << shuffle_size << std::endl;

    double average_timings[2] = {0, 0};

    for (u32 iteration = 0; iteration < iterations; ++iteration)
    {
        std::cout << std::setw(4) << iteration << ": " << std::flush;
        u32 timings[2] = {0, 0};

        auto crs = shuffle_proof::gen_CRS(shuffle_size);

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

        shuffle_proof::ProverContext prover_ctx(crs);
        shuffle_proof::VerifierContext verifier_ctx(crs);

        std::vector<u8> buffer;
        buffer.reserve(100000);
        bool continue_prover = true;
        bool continue_verifier = true;

        timer::start();
        shuffle_proof::begin(prover_ctx, c_old, c_new, keys.pk, permutation, rerandomization);
        timer::stop();
        timings[0] += timer::milliseconds();

        timer::start();
        shuffle_proof::begin(verifier_ctx, c_old, c_new, keys.pk);
        timer::stop();
        timings[1] += timer::milliseconds();

        while (continue_prover || continue_verifier)
        {
            if (continue_prover)
            {
                timer::start();
                continue_prover = shuffle_proof::step_prover(prover_ctx, buffer);
                timer::stop();
                timings[0] += timer::milliseconds();
            }
            if (continue_verifier)
            {
                timer::start();
                continue_verifier = shuffle_proof::step_verifier(verifier_ctx, buffer);
                timer::stop();
                timings[1] += timer::milliseconds();
            }
        }
        timer::start();
        bool result = shuffle_proof::get_result(verifier_ctx);
        timer::stop();
        timings[1] += timer::milliseconds();

        average_timings[0] += timings[0] / static_cast<double>(iterations);
        average_timings[1] += timings[1] / static_cast<double>(iterations);

        std::cout << "Prover: " << std::setw(4) << timings[0] << "ms, Verifier: " << std::setw(4) << timings[1] << "ms, Total: " << std::setw(4) << (timings[0] + timings[1]) << "ms" << std::endl;

        if (!result)
        {
            std::cout << "ERROR! Proof was not successful" << std::endl;
            return false;
        }
    }

    std::cout << "----------------------------" << std::endl;
    std::cout << "Final average:" << std::endl;
    std::cout << "    Prover: " << average_timings[0] << "ms, Verifier: " << average_timings[1] << "ms, Total: " << (average_timings[0] + average_timings[1]) << "ms" << std::endl
              << std::endl;

    return true;
}

bool benchmark_shuffle_proof(u32 iterations)
{
    return benchmark(iterations, 10) && benchmark(iterations, 100) && benchmark(iterations, 1000) && benchmark(iterations, 10000) && benchmark(iterations, 100000);
}
