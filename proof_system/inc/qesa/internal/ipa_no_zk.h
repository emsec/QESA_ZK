#pragma once

#include "qesa/commons.h"

namespace qesa
{
    namespace ipa_no_zk
    {
        struct ProverContext;
        struct VerifierContext;

        void begin(ProverContext& ctx, const CRS& crs, const math::vector<BN>& w_p, const math::vector<BN>& w_pp, const BN& t);
        void begin(VerifierContext& ctx, const CRS& crs, const G& c, const BN& t);

        // step_verifier has to be called first
        bool step_prover(ProverContext& ctx, std::vector<u8>& buffer);
        bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer);

        bool get_result(VerifierContext& ctx);

        struct ProverContext
        {
            CRS crs;
            math::vector<BN> w_p;
            math::vector<BN> w_pp;

            bool first_iteration;
            BN t;
            G u_minus_one, u_plus_one;
            BN v_minus_one, v_plus_one;

            u32 state;

            ProverContext() { state = 0; };
        };

        struct VerifierContext
        {
            CRS crs;
            G c;
            BN t;

            bool result;

            u32 state;

            VerifierContext() { state = 0; };
        };
    }
}
