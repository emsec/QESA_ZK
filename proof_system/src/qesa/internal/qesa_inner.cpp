#include "qesa/internal/qesa_inner.h"
#include <cmath>

namespace qesa::inner
{
    namespace
    {
        SparseMatrix batch(const std::vector<SparseMatrix>& matrices, const std::vector<BN>& x)
        {
            SparseMatrix res;
            for (u32 i = 0; i < matrices.size(); ++i)
            {
                for (auto [a, b] : matrices.at(i).vectors)
                {
                    if (a.non_zero_elements() < b.non_zero_elements())
                    {
                        a *= x[i];
                        a %= G::order();
                    }
                    else
                    {
                        b *= x[i];
                        b %= G::order();
                    }
                    res.vectors.emplace_back(a, b);
                }
            }
            return res;
        }

        math::vector<BN> compute_gammaT_times_sp(u32 n, SparseMatrix& gamma, math::vector<BN>& s_p)
        {
            // create gamma'
            {
                math::SparseVector<BN> a;
                math::SparseVector<BN> b;
                a.set(n - 1, 1);
                b.set(n - 2, 1);
                gamma.vectors.emplace_back(a, b);
            }
            {
                math::SparseVector<BN> a;
                math::SparseVector<BN> b;
                a.set(n - 2, 1);
                b.set(n - 1, -1);
                gamma.vectors.emplace_back(a, b);
            }


            math::vector<BN> gammaT_times_sp(n, BN(0));

            for (const auto& [a, b] : gamma.vectors)
            {
                auto tmp = b * ((a * s_p) % G::order());
                for (auto it = tmp.begin(); it != tmp.end(); ++it)
                {
                    gammaT_times_sp[it.index()] = (gammaT_times_sp.at(it.index()) + it.value()) % G::order();
                }
            }
            return gammaT_times_sp;
        }
    }

    void begin(ProverContext& ctx, const std::vector<SparseMatrix>& matrices, const math::vector<BN>& v_p)
    {
        ctx.matrices = matrices;
        ctx.v_p = v_p;
    }

    void begin(VerifierContext& ctx, const G& c, const std::vector<SparseMatrix>& matrices)
    {
        ctx.matrices = matrices;
        ctx.c_p = c;
    }

    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer)
    {
        // std::cout << "inner::step_prover " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            math::vector<BN> x;
            Deserializer deserializer(buffer);
            deserializer >> x;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            ctx.gamma = batch(ctx.matrices, x);

            auto& beta = x.at(1);
            ctx.crs.g_p[0] *= beta.mod_inverse(G::order());

            for (u32 i = 0; i < ctx.crs.n - 2; ++i)
            {
                ctx.v_pp.push_back(0);
            }

            math::vector<BN> w(ctx.v_p.begin(), ctx.v_p.end() - 2);

            for (const auto& [a, b] : ctx.gamma.vectors)
            {
                auto tmp = a * ((b * w) % G::order());
                for (auto it = tmp.begin(); it != tmp.end(); ++it)
                {
                    ctx.v_pp[it.index()] += it.value();
                }
            }

            ctx.v_pp.push_back(G::order() - ctx.v_p.at(ctx.crs.n - 1));
            ctx.v_pp.push_back(ctx.v_p.at(ctx.crs.n - 2));
            ctx.v_pp %= G::order();

            auto c_pp = ctx.crs.g_pp * ctx.v_pp;

            Serializer(buffer) << c_pp;

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            Deserializer deserializer(buffer);
            math::vector<BN> s;
            math::vector<BN> rand_b;
            deserializer >> s >> rand_b;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            math::vector<BN> s_p;
            s_p.insert(s_p.end(), s.begin(), s.end());
            s_p.insert(s_p.end(), rand_b.begin(), rand_b.end());

            math::vector<BN> gamma_times_s(ctx.crs.n - 2, BN(0));
            for (const auto& [a, b] : ctx.gamma.vectors)
            {
                auto tmp = a * ((b * s) % G::order());
                for (auto it = tmp.begin(); it != tmp.end(); ++it)
                {
                    gamma_times_s[it.index()] = (gamma_times_s.at(it.index()) + it.value()) % G::order();
                }
            }
            BN t = (-(gamma_times_s * s)) % G::order();

            auto gammaT_times_sp = compute_gammaT_times_sp(ctx.crs.n, ctx.gamma, s_p);

            ctx.v_p -= s_p;
            ctx.v_p %= G::order();

            ctx.v_pp += gammaT_times_sp;
            ctx.v_pp %= G::order();

            ipa_almost_zk::begin(ctx.ipa_almost_zk_ctx, ctx.crs, ctx.v_p, ctx.v_pp, t);
            ipa_almost_zk::step_prover(ctx.ipa_almost_zk_ctx, buffer);

            ctx.state = 2;
            return true;
        }
        else if (ctx.state == 2)
        {
            if (!ipa_almost_zk::step_prover(ctx.ipa_almost_zk_ctx, buffer))
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
        // std::cout << "inner::step_verifier " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            // std::cout << "engaging inner..." << std::endl;

            std::vector<BN> x;
            for (u32 i = 0; i < std::max((size_t)2, ctx.matrices.size()); ++i)
                //for (u32 i = 0; i < ctx.matrices.size(); ++i)
            {
                #if USE_SMALL_EXPONENTS
                x.push_back(BN::rand(SMALL_EXP_SIZE, true));
                #else
                x.push_back(BN::rand());
                #endif
            }
            ctx.gamma = batch(ctx.matrices, x);

            auto& beta = x.at(1);
            ctx.crs.g_p[0] *= beta.mod_inverse(G::order());
            ctx.c_p -= ctx.crs.g_p[0] * (beta - 1);

            Serializer serializer(buffer);
            serializer << x;

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            Deserializer deserializer(buffer);
            G c_pp;
            deserializer >> c_pp;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            math::vector<BN> s;
            for (u32 i = 0; i < ctx.crs.n - 2; ++i)
            {
                #if USE_SMALL_EXPONENTS
                s.push_back(BN::rand(SMALL_EXP_SIZE, true));
                #else
                s.push_back(BN::rand());
                #endif
            }
            #if USE_SMALL_EXPONENTS
            math::vector<BN> rand_b = {BN::rand(SMALL_EXP_SIZE, true), BN::rand(SMALL_EXP_SIZE, true)};
            #else
            math::vector<BN> rand_b = {BN::rand(), BN::rand()};
            #endif

            Serializer(buffer) << s << rand_b;

            math::vector<BN> s_p(s.begin(), s.end());
            s_p.insert(s_p.end(), rand_b.begin(), rand_b.end());

            math::vector<BN> gamma_times_s(ctx.crs.n - 2, BN(0));

            for (const auto& [a, b] : ctx.gamma.vectors)
            {
                auto tmp = a * ((b * s) % G::order());
                for (auto it = tmp.begin(); it != tmp.end(); ++it)
                {
                    gamma_times_s[it.index()] = (gamma_times_s.at(it.index()) + it.value()) % G::order();
                }
            }
            BN t = (-(gamma_times_s * s)) % G::order();

            auto gammaT_times_sp = compute_gammaT_times_sp(ctx.crs.n, ctx.gamma, s_p);

            auto c = ctx.c_p - ctx.crs.g_p * s_p + c_pp + ctx.crs.g_pp * gammaT_times_sp;

            ipa_almost_zk::begin(ctx.ipa_almost_zk_ctx, ctx.crs, c, t);

            ctx.state = 2;
            return true;
        }
        else if (ctx.state == 2)
        {
            if (!ipa_almost_zk::step_verifier(ctx.ipa_almost_zk_ctx, buffer))
            {
                ctx.state = 3;
                return false;
            }
            return true;
        }
        throw std::runtime_error("reached unknown state");
    }

    bool get_result(VerifierContext& ctx)
    {
        return ipa_almost_zk::get_result(ctx.ipa_almost_zk_ctx);
    }
}
