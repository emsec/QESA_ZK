#include "test.h"
#include "qesa/qesa_range_proof.h"
#include "timer.h"

using namespace Group;

bool test_qesa_range_proof()
{

    std::cout << "Testing QESA range proof..." << std::endl;

    u32 bitrange = 60;
    u32 num_values = 2;

    qesa::CRS crs = qesa::gen_CRS(2 + bitrange * num_values);

    std::cout << "CRS size: " << crs.n << std::endl;

    std::vector<std::tuple<BN, BN>> openings;
    std::vector<G> commitments;

    for (u32 i = 0; i < num_values; ++i)
    {
        BN v = BN::rand(bitrange - 1, true);
        BN r = BN::rand();
        openings.push_back({v, r});
        commitments.push_back(qesa::range_proof::commit(crs, v, r));
    }


    qesa::range_proof::ProverContext pctx(crs);
    qesa::range_proof::VerifierContext vctx(crs);

    std::vector<u8> buffer;
    buffer.reserve(10000);

    bool continue_prover = true;
    bool continue_verifier = true;

    timer::start();

    qesa::range_proof::begin(pctx, bitrange, openings);
    qesa::range_proof::begin(vctx, bitrange, commitments);

    while (continue_prover || continue_verifier)
    {
        if (continue_prover)
        {
            continue_prover = qesa::range_proof::step_prover(pctx, buffer);
        }
        if (continue_verifier)
        {
            continue_verifier = qesa::range_proof::step_verifier(vctx, buffer);
        }
    }
    auto result = qesa::range_proof::get_result(vctx);
    timer::stop();
    std::cout << timer::milliseconds() << "ms, " << std::boolalpha << "result = " << result << std::endl << std::endl;
    if (!result) return false;
    return true;
}
