#pragma once

#include "qesa/qesa_copy.h"

/*
 * QESA aggregate range proofs
 *
 * Proves that m commited values v are in the range {0, ..., 2^n - 1}.
 * CRS witness size has to be at least 2 + m*n.
 *
 * Prover input: The range n and m commitment openings (value, randomness)
 * Verifier input: The range n and m commitments
 */

namespace qesa::range_proof
{
    using namespace Group;

    struct ProverContext;
    struct VerifierContext;

    // ##################################
    // methods
    // ##################################

    G commit(const CRS& crs, const BN& m, const BN& r);

    void begin(ProverContext& ctx, u32 range, const std::vector<std::tuple<BN,BN>>& openings);
    void begin(VerifierContext& ctx, u32 range, const std::vector<G>& commitments);

    /*
     * Protocol execution:
     * step_prover and step_verifier have to be called alternatingly, step_prover has to be called first.
     * Both functions take their respective context and a buffer for the network packet as input.
     * The buffer is read by the function and filled with the new outputs.
     * Both functions return whether the protocol is still running.
     */
    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer);
    bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer);

    bool get_result(VerifierContext& ctx);

    // ##################################
    // types
    // ##################################

    struct ProverContext
    {
        const CRS& crs;

        u32 range;
        std::vector<std::tuple<BN,BN>> openings;

        BN r[2];
        G a;

        u32 state;

        qesa::copy::ProverContext qesa_copy_ctx;

        ProverContext(const CRS& p_crs) : crs(p_crs), qesa_copy_ctx(p_crs) { state = 0; };
    };

    struct VerifierContext
    {
        const CRS& crs;

        u32 range;
        std::vector<G> commitments;

        G rhs;

        bool result;
        u32 state;

        qesa::copy::VerifierContext qesa_copy_ctx;

        VerifierContext(const CRS& p_crs) : crs(p_crs), qesa_copy_ctx(p_crs) { state = 0; };
    };
}
