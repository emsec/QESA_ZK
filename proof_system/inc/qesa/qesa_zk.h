#pragma once

#include "qesa/internal/qesa_inner.h"

/*
 * QESA zero-knowledge proof
 *
 * Proves that Gamma_i * witness == 0 for all i.
 * CRS witness size has to be at least witness.size().
 * witness[0] has to be 1.
 *
 * Prover input: Matrices Gamma_i, corresponding witness
 * Verifier input: Matrices Gamma_i
 */

namespace qesa::qesa_zk
{
    struct ProverContext;
    struct VerifierContext;

    // ##################################
    // methods
    // ##################################

    /*
     * Protocol initiation:
     * Prover and verifier have to initialize their contexts with these functions.
     */
    void begin(ProverContext& ctx, const std::vector<SparseMatrix>& matrices, const math::vector<BN>& witness);
    void begin(VerifierContext& ctx, const std::vector<SparseMatrix>& matrices);

    /*
     * Protocol execution:
     * step_prover and step_verifier have to be called alternatingly, step_prover has to be called first.
     * Both functions take their respective context and a buffer for the network packet as input.
     * The buffer is read by the function, cleared, and filled with the new outputs.
     * Both functions return true if the protocol is still running, i.e., if they need to be called again on the partners response.
     */
    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer);
    bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer);

    /*
     * Given a VerifierContext after protocol execution, this returns true if the proof was correct.
     */
    bool get_result(VerifierContext& ctx);

    // ##################################
    // types
    // ##################################

    struct ProverContext : inner::ProverContext
    {
        using inner::ProverContext::ProverContext;

        u32 state;
    };

    struct VerifierContext : inner::VerifierContext
    {
        using inner::VerifierContext::VerifierContext;

        u32 state;
    };
}
