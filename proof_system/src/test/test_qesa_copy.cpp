#include "test.h"
#include "qesa/qesa_copy.h"
#include "timer.h"

using namespace Group;

bool test_qesa_copy()
{
    std::cout << "testing QESA Copy" << std::endl;
    std::vector<qesa::SparseMatrix> matrices;

    // setup witness
    math::vector<BN> witness = {1};

    // setup CRS and commitment CRSs
    auto crs = qesa::gen_CRS(1 + 14);
    auto com_crs_1 = qesa::copy::get_commitment_crs(crs, {1, 2});
    auto com_crs_2 = qesa::copy::get_commitment_crs(crs, {2, 4, 7});

    // commitment messages
    std::vector<BN> m1 = {BN::rand(), BN::rand()};
    BN r1 = BN::rand();

    std::vector<BN> m2 = {BN::rand(), BN::rand(), BN::rand()};
    BN r2 = BN::rand();

    std::vector<BN> m3 = {BN::rand(), BN::rand()};
    BN r3 = BN::rand();

    // commit to messages
    auto com_1 = qesa::copy::commit(crs, com_crs_1, m1, r1);
    auto com_2 = qesa::copy::commit(crs, com_crs_2, m2, r2);
    auto com_3 = qesa::copy::commit(crs, com_crs_1, m3, r3);

    // create qesa::copy mapping
    auto mapping = qesa::copy::compute_mapping(crs, 1, {com_crs_1, com_crs_2, com_crs_1});

    // execute qesa::copy
    qesa::copy::ProverContext pctx(crs);
    qesa::copy::VerifierContext vctx(crs);

    std::vector<u8> buffer;
    buffer.reserve(10000);

    bool continue_prover = true;
    bool continue_verifier = true;

    timer::start();

    {
        std::vector<std::tuple<std::vector<BN>, BN>> commitments;
        commitments.emplace_back(m1, r1);
        commitments.emplace_back(m2, r2);
        commitments.emplace_back(m3, r3);
        qesa::copy::begin(pctx, matrices, mapping, witness, commitments);
    }

    qesa::copy::begin(vctx, matrices, mapping, {com_1, com_2, com_3});


    while (continue_prover || continue_verifier)
    {
        if (continue_prover)
        {
            continue_prover = qesa::copy::step_prover(pctx, buffer);
        }
        if (continue_verifier)
        {
            continue_verifier = qesa::copy::step_verifier(vctx, buffer);
        }
    }
    auto result = qesa::copy::get_result(vctx);
    timer::stop();
    std::cout << timer::milliseconds() << "ms, " << std::boolalpha << "result = " << result << std::endl << std::endl;

    return result;
}
