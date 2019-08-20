#pragma once

#include "bulletproofs/commons.h"

namespace bulletproofs::ipa
{
    using namespace Group;

    struct ProverContext;
    struct VerifierContext;


    // ##################################
    // methods
    // ##################################

    void begin(ProverContext& ctx, const CRS& crs, const math::vector<BN>& a, const math::vector<BN>& b);
    void begin(VerifierContext& ctx, const CRS& crs, const G& P, const BN& c);

    // step_verifier has to be called first
    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer);
    bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer);

    bool get_result(VerifierContext& ctx);


    // ##################################
    // contexts
    // ##################################

    struct ProverContext
    {
        CRS crs;

        bool first_iteration;
        math::vector<BN> a;
        math::vector<BN> b;

        u32 state;

        ProverContext() { state = 0; };
    };

    struct VerifierContext
    {
        CRS crs;

        G P;
        BN c;
        bool result;

        u32 state;

        VerifierContext() { state = 0; result = false; };
    };
}
