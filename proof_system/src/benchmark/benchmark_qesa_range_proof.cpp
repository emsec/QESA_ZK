#include "benchmark.h"
#include "bulletproofs/range_proof.h"
#include "qesa/qesa_range_proof.h"

using namespace Group;

static bool benchmark(u32 iterations, u32 bitrange, u32 num_commitments)
{
    std::cout << "Benchmarking QESA range proof for " << num_commitments << " " << bitrange << "-bit values" << std::endl;
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

        qesa::CRS crs = qesa::gen_CRS(2 + bitrange * num_commitments);

        std::vector<G> commitments;
        for (const auto &[v, r] : openings)
        {
            commitments.push_back(qesa::range_proof::commit(crs, v, r));
        }

        qesa::range_proof::ProverContext pctx(crs);
        qesa::range_proof::VerifierContext vctx(crs);

        std::vector<u8> buffer;
        buffer.reserve(100000);

        bool continue_prover = true;
        bool continue_verifier = true;

        timer::start();
        qesa::range_proof::begin(pctx, bitrange, openings);
        timer::stop();
        timings[0] += timer::milliseconds();

        timer::start();
        qesa::range_proof::begin(vctx, bitrange, commitments);
        timer::stop();
        timings[1] += timer::milliseconds();

        while (continue_prover || continue_verifier)
        {
            if (continue_prover)
            {
                timer::start();
                continue_prover = qesa::range_proof::step_prover(pctx, buffer);
                timer::stop();
                timings[0] += timer::milliseconds();
            }
            if (continue_verifier)
            {
                timer::start();
                continue_verifier = qesa::range_proof::step_verifier(vctx, buffer);
                timer::stop();
                timings[1] += timer::milliseconds();
            }
        }
        timer::start();
        auto result = qesa::range_proof::get_result(vctx);
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

bool benchmark_qesa_range_proof(u32 iterations)
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
