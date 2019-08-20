#pragma once

#include "qesa/internal/ipa_almost_zk.h"

namespace qesa::inner
{
    struct ProverContext;
    struct VerifierContext;

    void begin(ProverContext& ctx, const std::vector<SparseMatrix>& matrices, const math::vector<BN>& v_p);
    void begin(VerifierContext& ctx, const G& c, const std::vector<SparseMatrix>& matrices);

    // step_verifier has to be called first
    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer);
    bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer);

    bool get_result(VerifierContext& ctx);

    struct ProverContext
    {
        CRS crs;
        std::vector<SparseMatrix> matrices;

        SparseMatrix gamma;

        math::vector<BN> v_p;
        math::vector<BN> v_pp;
        ipa_almost_zk::ProverContext ipa_almost_zk_ctx;

        u32 state;

        ProverContext(const CRS& p_crs): crs(p_crs) { state = 0; };
    };

    struct VerifierContext
    {
        CRS crs;

        std::vector<SparseMatrix> matrices;
        G c_p;
        SparseMatrix gamma;
        ipa_almost_zk::VerifierContext ipa_almost_zk_ctx;

        u32 state;

        VerifierContext(const CRS& p_crs): crs(p_crs) { state = 0;};
    };

}
