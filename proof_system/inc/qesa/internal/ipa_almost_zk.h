#pragma once

#include "qesa/internal/ipa_no_zk.h"

namespace qesa
{
    namespace ipa_almost_zk
    {
        struct ProverContext;
        struct VerifierContext;

        void begin(ProverContext& ctx, const CRS& crs, const math::vector<BN>& w_p, const math::vector<BN>& w_pp, const BN& t);
        void begin(VerifierContext& ctx, const CRS& crs, const G& c, const BN& t);

        // step prover has to be called first
        bool step_prover(ProverContext& ctx, std::vector<u8>& buffer);
        bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer);

        bool get_result(VerifierContext& ctx);

        struct ProverContext
        {
            CRS crs;
            math::vector<BN> w_p;
            math::vector<BN> w_pp;
            BN t;

            G c_r;
            math::vector<BN> r_p;
            math::vector<BN> r_pp;

            ipa_no_zk::ProverContext ipa_no_zk_ctx;

            u32 state;

            ProverContext() { state = 0; };
        };

        struct VerifierContext
        {
            CRS crs;
            G c;
            BN t;

            ipa_no_zk::VerifierContext ipa_no_zk_ctx;

            u32 state;

            VerifierContext() { state = 0; };
        };
    }
}
