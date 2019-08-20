#include "qesa/internal/ipa_almost_zk.h"

namespace qesa::ipa_almost_zk
{
    using namespace Group;

    namespace
    {
        math::vector<BN> sample_M_n_plus(u32 n)
        {
            std::vector<BN> r(n, BN(0));
            r[0] = BN::rand();
            r[1] = BN::rand();
            for (u32 i = 4; i <= n; i *= 2)
            {
                r[i / 2] = BN::rand();
                r[i / 2 + 1] = BN::rand();
            }
            r[n - 2] = BN::rand();
            r[n - 1] = BN::rand();
            return math::vector<BN>(r.begin(), r.end());
        }

        math::vector<BN> orth(const math::vector<BN>& y, const math::vector<BN>& x)
        {
            auto yy = y * y;
            if (yy == 0)
            {
                throw std::runtime_error("Gram-Schmidt sampling: y^2 is zero. Please retry.");
            }
            auto f = (x * y) % G::order();
            f = (f * yy.mod_inverse(G::order())) % G::order();
            return (x - y * f) % G::order();
        }

        math::vector<BN> sample_gram_schmidt(const math::vector<BN>& w)
        {
            return orth(w, sample_M_n_plus(w.size()));
        }

        math::vector<BN> sample_gram_schmidt(const math::vector<BN>& w, const math::vector<BN>& s)
        {
            return orth(orth(w, s), orth(w, sample_M_n_plus(w.size())));
        }
    }

    void begin(ProverContext& ctx, const CRS& crs, const math::vector<BN>& w_p, const math::vector<BN>& w_pp, const BN& t)
    {
        ctx.crs = crs;
        ctx.w_p = w_p;
        ctx.w_pp = w_pp;
        ctx.t = t;
    }

    void begin(VerifierContext& ctx, const CRS& crs, const G& c, const BN& t)
    {
        ctx.crs = crs;
        ctx.c = c;
        ctx.t = t;
    }

    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer)
    {
        if (ctx.state == 0)
        {
            // std::cout << "engaging IPA_almost_zk..." << std::endl;

            ctx.r_p = sample_gram_schmidt(ctx.w_pp);
            ctx.r_pp = sample_gram_schmidt(ctx.w_p, ctx.r_p);

            ctx.c_r = ctx.crs.g_p * ctx.r_p + ctx.crs.g_pp * ctx.r_pp;

            Serializer(buffer) << ctx.c_r;

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            Deserializer deserializer(buffer);
            BN beta;
            deserializer >> beta;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            auto w_p_new = (ctx.w_p * beta + ctx.r_p) % G::order();
            auto w_pp_new = (ctx.w_pp * beta + ctx.r_pp) % G::order();

            ctx.t = (((beta * beta) % G::order()) * ctx.t) % G::order();

            ipa_no_zk::begin(ctx.ipa_no_zk_ctx, ctx.crs, w_p_new, w_pp_new, ctx.t);

            ctx.state = 2;
            return true;
        }
        else if (ctx.state == 2)
        {
            if (!ipa_no_zk::step_prover(ctx.ipa_no_zk_ctx, buffer))
            {
                ctx.state = 3;
                return false;
            }
            return true;
        }
        throw std::runtime_error("reached unknown state");
    }

    bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer)
    {
        if (ctx.state == 0)
        {
            Deserializer deserializer(buffer);
            G c_r;
            deserializer >> c_r;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            #if USE_SMALL_EXPONENTS
            auto beta = BN::rand(SMALL_EXP_SIZE, true);
            #else
            auto beta = BN::rand();
            #endif

            ctx.t = (((beta * beta) % G::order()) * ctx.t) % G::order();

            ctx.c = ctx.c * beta + c_r + ctx.crs.Q * ctx.t;

            Serializer(buffer) << beta;

            ipa_no_zk::begin(ctx.ipa_no_zk_ctx, ctx.crs, ctx.c, ctx.t);

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            if (!ipa_no_zk::step_verifier(ctx.ipa_no_zk_ctx, buffer))
            {
                ctx.state = 2;
                return false;
            }
            return true;
        }
        throw std::runtime_error("reached unknown state");
    }

    bool get_result(VerifierContext& ctx)
    {
        return ipa_no_zk::get_result(ctx.ipa_no_zk_ctx);
    }
}
