#pragma once

#include "bulletproofs/commons.h"

#if USE_BULLETPROOFS_IPA
    #include "bulletproofs/internal/ipa.h"
#endif

#if USE_QESA_IPA
    #include "qesa/internal/ipa_no_zk.h"
#endif

/*
 * Bulletproofs aggregate range proofs
 *
 * Proves that m committed values are in the range {0, ..., 2^n - 1}.
 *
 * Prover input: m commitment openings (value, randomness)
 * Verifier input: m commitments
 */

namespace bulletproofs
{
    using namespace Group;

    struct ProverContext;
    struct VerifierContext;

    // ##################################
    // methods
    // ##################################

    void begin(ProverContext& ctx, const std::vector<std::tuple<BN, BN>>& openings);
    void begin(VerifierContext& ctx, const std::vector<G>& commitments);

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
        CRS crs;

        std::vector<std::tuple<BN, BN>> openings;

        BN y;
        BN z;
        math::vector<BN> a_L, a_R;
        math::vector<BN> s_L, s_R;
        BN alpha;
        BN rho;
        BN tau_1;
        BN tau_2;

        math::vector<BN> l_0, l_1;
        math::vector<BN> r_0, r_1;

        #if USE_BULLETPROOFS_IPA
        bulletproofs::ipa::ProverContext ipa_ctx;
        #endif

        #if USE_QESA_IPA
        qesa::ipa_no_zk::ProverContext ipa_ctx;
        #endif

        u32 state;

        ProverContext(const CRS& p_crs): crs(p_crs) { state = 0; };
    };

    struct VerifierContext
    {
        CRS crs;

        std::vector<G> commitments;

        G A;
        G S;
        BN x;
        BN y;
        BN z;
        G T_1;
        G T_2;

        #if USE_BULLETPROOFS_IPA
        bulletproofs::ipa::VerifierContext ipa_ctx;
        #endif

        #if USE_QESA_IPA
        qesa::ipa_no_zk::VerifierContext ipa_ctx;
        #endif

        bool result;

        u32 state;

        VerifierContext(const CRS& p_crs): crs(p_crs) { state = 0; result = false; };
    };
}
