#include "qesa/internal/ipa_no_zk.h"

namespace qesa::ipa_no_zk
{
    using namespace Group;

    void begin(ProverContext& ctx, const CRS& crs, const math::vector<BN>& w_p, const math::vector<BN>& w_pp, const BN& t)
    {
        ctx.crs = crs;
        ctx.w_p = w_p;
        ctx.w_pp = w_pp;
        ctx.first_iteration = true;
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
            // std::cout << "engaging IPA_no_zk..." << std::endl;

            Deserializer deserializer(buffer);
            BN alpha;
            deserializer >> alpha;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            ctx.crs.Q *= alpha.mod_inverse(G::order());

            ctx.state = 1;
        }
        // no else if!
        if (ctx.state == 1)
        {
            if (!ctx.first_iteration)
            {
                Deserializer deserializer(buffer);
                BN xi;
                deserializer >> xi;
                if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

                for (u32 i = 0; i < ctx.crs.n / 2; ++i)
                {
                    ctx.crs.g_p[i] += ctx.crs.g_p.at(ctx.crs.n / 2 + i) * xi;
                    ctx.crs.g_pp[i] = ctx.crs.g_pp.at(i) * xi + ctx.crs.g_pp.at(ctx.crs.n / 2 + i);

                    ctx.w_p[i] = (ctx.w_p.at(i) * xi + ctx.w_p.at(ctx.crs.n / 2 + i)) % G::order();
                    ctx.w_pp[i] = (ctx.w_pp.at(i) + ctx.w_pp.at(ctx.crs.n / 2 + i) * xi) % G::order();
                }

                auto xi_squared = (xi * xi) % G::order();

                ctx.t = (ctx.v_plus_one * xi_squared + ctx.t * xi + ctx.v_minus_one) % G::order();

                ctx.crs.n /= 2;
            }

            if (ctx.crs.n == 1)
            {
                Serializer(buffer) << ctx.w_p.at(0) << ctx.w_pp.at(0) << ctx.t;
                ctx.state = 2;
                return false;
            }

            ctx.first_iteration = false;

            G u_minus_one_p = 0;
            G u_plus_one_p = 0;
            G u_minus_one_pp = 0;
            G u_plus_one_pp = 0;
            ctx.v_minus_one = 0;
            ctx.v_plus_one = 0;
            for (u32 i = 0; i < ctx.crs.n / 2; ++i)
            {
                u_minus_one_p += ctx.crs.g_p.at(ctx.crs.n / 2 + i) * ctx.w_p.at(i);
                u_plus_one_p += ctx.crs.g_p.at(i) * ctx.w_p.at(ctx.crs.n / 2 + i);
                u_minus_one_pp += ctx.crs.g_pp.at(ctx.crs.n / 2 + i) * ctx.w_pp.at( i);
                u_plus_one_pp += ctx.crs.g_pp.at(i) * ctx.w_pp.at(ctx.crs.n / 2 + i);
            }

            for (u32 i = 0; i < ctx.crs.n / 2; ++i)
            {
                ctx.v_minus_one += ctx.w_p.at(ctx.crs.n / 2 + i) * ctx.w_pp.at(i);
                ctx.v_plus_one += ctx.w_p.at(i) * ctx.w_pp.at(ctx.crs.n / 2 + i);
            }
            ctx.v_minus_one %= G::order();
            ctx.v_plus_one %= G::order();

            ctx.u_minus_one = u_minus_one_p + u_plus_one_pp + ctx.crs.Q * ctx.v_plus_one;
            ctx.u_plus_one = u_plus_one_p + u_minus_one_pp + ctx.crs.Q * ctx.v_minus_one;

            Serializer(buffer) << ctx.u_minus_one << ctx.u_plus_one;

            return true;
        }
        throw std::runtime_error("reached unknown state");
    }

    bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer)
    {
        if (ctx.state == 0)
        {
            #if USE_SMALL_EXPONENTS
            auto alpha = BN::rand(SMALL_EXP_SIZE, true);
            #else
            auto alpha = BN::rand();
            #endif

            ctx.crs.Q *= alpha.mod_inverse(G::order());
            ctx.c -= ctx.crs.Q * (((alpha - 1) * ctx.t) % G::order());

            Serializer(buffer) << alpha;

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            if (ctx.crs.n == 1)
            {
                BN w_p, w_pp;
                BN t;
                Deserializer deserializer(buffer);
                deserializer >> w_p >> w_pp >> t;
                if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

                bool cond_1 = (ctx.c == ctx.crs.g_p.at(0) * w_p + ctx.crs.g_pp.at(0) * w_pp + ctx.crs.Q * t);
                bool cond_2 = (t == ((w_p * w_pp) % G::order()));

                // std::cout << std::endl << std::boolalpha;
                // std::cout << "condition 1: " << cond_1 << std::endl;
                // std::cout << "condition 2: " << cond_2 << std::endl;

                ctx.result = cond_1 && cond_2;

                ctx.state = 2;
                return false;
            }
            else
            {
                G u_minus_one, u_plus_one;
                Deserializer deserializer(buffer);
                deserializer >> u_minus_one >> u_plus_one;
                if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

                #if USE_SMALL_EXPONENTS
                auto xi = BN::rand(SMALL_EXP_SIZE, true);
                #else
                auto xi = BN::rand();
                #endif

                Serializer(buffer) << xi;

                for (u32 i = 0; i < ctx.crs.n / 2; ++i)
                {
                    ctx.crs.g_p[i] += ctx.crs.g_p.at(ctx.crs.n / 2 + i) * xi;
                    ctx.crs.g_pp[i] = ctx.crs.g_pp.at(i) * xi + ctx.crs.g_pp.at(ctx.crs.n / 2 + i);
                }

                auto xi_squared = (xi * xi) % G::order();

                ctx.c = u_minus_one * xi_squared + ctx.c * xi + u_plus_one;

                ctx.crs.n /= 2;
            }
            return true;
        }
        throw std::runtime_error("reached unknown state");
    }

    bool get_result(VerifierContext& ctx)
    {
        return ctx.result;
    }

}
