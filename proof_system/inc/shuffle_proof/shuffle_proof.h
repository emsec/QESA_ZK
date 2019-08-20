#pragma once

#include "settings.h"

#include "qesa/internal/qesa_inner.h"
#include "lmpa/lmpa_simple_zk.h"
#include "shuffle_proof/el_gamal.h"

/*
 * Shuffle proof using QESA and LMPA
 *
 * Given two vectors of ElGamal encryptions c_old and c_new, this proves that
 *   c_new is a rerandomized permutation of c_old.
 *
 * Prover input: c_old, c_new, ElGamal public key, the permutation indices, and the rerandomization scalars
 * Verifier input: c_old, c_new and ElGamal public key
 */

namespace shuffle_proof
{
    using namespace Group;

    struct CRS;
    struct ProverContext;
    struct VerifierContext;

    // ##################################
    // methods
    // ##################################

    CRS gen_CRS(u32 shuffle_size);


    void begin(ProverContext& ctx,
               const std::vector<el_gamal::Ciphertext>& c_old,
               const std::vector<el_gamal::Ciphertext>& c_new,
               const el_gamal::PublicKey& pk,
               const std::vector<u32>& perm,
               const math::vector<BN>& rerandomization);

    void begin(VerifierContext& ctx,
               const std::vector<el_gamal::Ciphertext>& c_old,
               const std::vector<el_gamal::Ciphertext>& c_new,
               const el_gamal::PublicKey& pk);

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

    struct CRS
    {
        qesa::CRS qesa_crs;
        u32 shuffle_size;
    };

    struct ProverContext
    {
        const CRS& crs;

        std::vector<el_gamal::Ciphertext> c_old;
        std::vector<el_gamal::Ciphertext> c_new;
        el_gamal::PublicKey pk;
        std::vector<u32> pi;
        math::vector<BN> rerandomization;

        math::vector<BN> x;
        math::vector<BN> y;

        G com_pi;
        G com_y;
        BN r_pi;
        BN r_y;

        BN xi;
        BN z;

        math::vector<BN> d;

        qesa::inner::ProverContext qesa_inner_ctx;

        BN sigma;
        BN r_sigma;
        G com_sigma;
        lmpa::simple_zk::ProverContext lmpa_ctx;

        u32 state;

        ProverContext(const CRS& p_crs): crs(p_crs), qesa_inner_ctx(p_crs.qesa_crs) { state = 0; };
    };

    struct VerifierContext
    {
        const CRS& crs;

        std::vector<el_gamal::Ciphertext> c_old;
        std::vector<el_gamal::Ciphertext> c_new;
        el_gamal::PublicKey pk;

        std::vector<BN> x;

        G com_pi;
        G com_y;

        BN xi;
        BN z;

        qesa::inner::VerifierContext qesa_inner_ctx;
        lmpa::simple_zk::VerifierContext lmpa_ctx;

        u32 state;

        bool result;

        VerifierContext(const CRS& p_crs): crs(p_crs), qesa_inner_ctx(p_crs.qesa_crs) { state = 0; };
    };
}
