#include "benchmark.h"
#include "bulletproofs/range_proof.h"
#include "qesa/qesa_range_proof.h"

using namespace Group;

static bool benchmark(u32 iterations, u32 bitrange, u32 num_commitments)
{
    std::cout << "Benchmarking bulletproofs for " << num_commitments << " " << bitrange << "-bit values" << std::endl;
    if (bitrange > G::order().bitlength())
    {
        std::cout << bitrange << " bit range is not supported (max " << G::order().bitlength() << ")" << std::endl;
        return false;
    }

    double average_timings[2] = {0, 0};

    for (u32 iteration = 0; iteration < iterations; ++iteration)
    {
        std::cout << std::setw(4) << iteration << ": " << std::flush;
        u32 timings[2] = {0, 0};

        std::vector<std::tuple<BN, BN>> openings;
        for (u32 i = 0; i < num_commitments; ++i)
        {
            auto v = BN::rand(bitrange - 1, true);
            auto r = BN::rand();
            openings.emplace_back(std::make_tuple(v, r));
        }

        COM::CRS com_crs = COM::setup(1);
        bulletproofs::CRS crs = bulletproofs::setup(bitrange, num_commitments, com_crs);

        bulletproofs::ProverContext prover_ctx(crs);
        bulletproofs::VerifierContext verifier_ctx(crs);

        std::vector<u8> buffer;
        buffer.reserve(100000);

        bool continue_prover = true;
        bool continue_verifier = true;

        std::vector<G> commitments;
        for (const auto &[v, r] : openings)
        {
            commitments.push_back(COM::commit(com_crs, {v}, r));
        }

        timer::start();
        bulletproofs::begin(prover_ctx, openings);
        timer::stop();
        timings[0] += timer::milliseconds();

        timer::start();
        bulletproofs::begin(verifier_ctx, commitments);
        timer::stop();
        timings[1] += timer::milliseconds();

        while (continue_prover || continue_verifier)
        {
            if (continue_prover)
            {
                timer::start();
                continue_prover = bulletproofs::step_prover(prover_ctx, buffer);
                timer::stop();
                timings[0] += timer::milliseconds();
            }
            if (continue_verifier)
            {
                timer::start();
                continue_verifier = bulletproofs::step_verifier(verifier_ctx, buffer);
                timer::stop();
                timings[1] += timer::milliseconds();
            }
        }
        timer::start();
        auto result = bulletproofs::get_result(verifier_ctx);
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

bool benchmark_bulletproofs(u32 iterations)
{
    bool result = true;

    if (result)
    {
        result &= benchmark(iterations, 60, 1) && benchmark(iterations, 60, 2) && benchmark(iterations, 60, 32) && benchmark(iterations, 60, 128) && benchmark(iterations, 60, 512);
    }
    if (result)
    {
        result &= benchmark(iterations, 124, 1) && benchmark(iterations, 124, 2) && benchmark(iterations, 124, 32) && benchmark(iterations, 124, 128) && benchmark(iterations, 124, 512);
    }
    if (result)
    {
        result &= benchmark(iterations, 252, 1) && benchmark(iterations, 252, 2) && benchmark(iterations, 252, 32) && benchmark(iterations, 252, 128) && benchmark(iterations, 252, 512);
    }

    return result;
}
