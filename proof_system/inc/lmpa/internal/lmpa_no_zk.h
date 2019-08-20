#pragma once

#include "lmpa/commons.h"

namespace lmpa::no_zk
{
    using namespace Group;

    struct ProverContext;
    struct VerifierContext;


    // ##################################
    // methods
    // ##################################

    void begin(ProverContext& ctx, const Matrix<G>& A, const math::vector<BN>& w);
    void begin(VerifierContext& ctx, const Matrix<G>& A, const math::vector<G>& t);

    // step_prover has to be called first
    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer);
    bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer);

    bool get_result(VerifierContext& ctx);


    // ##################################
    // contexts
    // ##################################

    struct ProverContext
    {
        bool first_iteration;

        Matrix<G> A;
        math::vector<BN> w;

        u32 n;
        math::vector<G> u_minus_one;
        math::vector<G> u_plus_one;

        u32 state;

        ProverContext() { state = 0; };
    };

    struct VerifierContext
    {
        Matrix<G> A;
        math::vector<G> t;

        u32 n;
        bool result;

        u32 state;

        VerifierContext() { state = 0; result = false; };
    };
}
