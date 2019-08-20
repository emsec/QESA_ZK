#pragma once

#include "lmpa/internal/lmpa_no_zk.h"

/*
 * LMPA simple zero-knowledge variant
 *
 * Proves that A*w == t (no batching applied).
 *
 * Prover input: Matrix A and witness vector w
 * Verifier input: Matrix A and result vector t
 */

namespace lmpa::simple_zk
{
    using namespace Group;

    struct ProverContext;
    struct VerifierContext;

    // ##################################
    // methods
    // ##################################

    void begin(ProverContext& ctx, const Matrix<G>& A, const math::vector<BN>& w);
    void begin(VerifierContext& ctx, const Matrix<G>& A, const math::vector<G>& t);

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
    // contexts
    // ##################################

    struct ProverContext: no_zk::ProverContext
    {
        math::vector<BN> r;
        math::vector<G> a;

        u32 state;

        ProverContext() { state = 0; };
    };

    struct VerifierContext: no_zk::VerifierContext
    {
        u32 state;

        VerifierContext() { state = 0; };
    };
}
