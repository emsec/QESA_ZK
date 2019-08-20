#include "bulletproofs/range_proof.h"
#include "math/serializer.h"
#include "math/deserializer.h"

namespace bulletproofs
{
    namespace
    {
        BN delta(const BN& y, const BN& z, u32 n, u32 m)
        {
            auto d = BN(0);
            auto f_y = BN(1);
            for (u32 i = 0; i < n * m; ++i)
            {
                d += f_y;
                if (i != n * m - 1) f_y = (f_y * y) % G::order();
            }
            d %= G::order();
            auto zz = (z * z) % G::order();
            d = ((z - zz) * d) % G::order();

            auto all_one = BN(0);
            for (u32 i = 0; i < n; ++i)
            {
                all_one <<= 1;
                all_one += 1;
            }

            for (u32 i = 0; i < m; ++i)
            {
                zz = (zz * z) % G::order();
                d -= zz * all_one;
            }

            return d % G::order();
        }

        template <class T>
        void mix(math::vector<T>& v, const BN& x_L, const BN& x_R)
        {
            u32 n = v.size() / 2;
            for (u32 i = 0; i < n; ++i)
            {
                v[i] = v.at(i) * x_L + v.at(n + i) * x_R;
            }
            v.resize(n);
        }
    }
    void begin(ProverContext& ctx, const std::vector<std::tuple<BN, BN>>& openings)
    {
        ctx.openings = openings;
    }

    void begin(VerifierContext& ctx, const std::vector<G>& commitments)
    {
        ctx.commitments = commitments;
    }

    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer)
    {
        // std::cout << "bulletproofs::step_prover " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            ctx.alpha = BN::rand();
            auto A = G::get_gen() * ctx.alpha;

            ctx.a_L.reserve(ctx.crs.n * ctx.crs.m);
            ctx.a_R.reserve(ctx.crs.n * ctx.crs.m);
            auto minus_one = G::order() - 1;

            for (u32 j = 0; j < ctx.crs.m; ++j)
            {
                auto v = std::get<0>(ctx.openings.at(j));
                for (u32 i = 0; (i < v.bitlength()) && (i < ctx.crs.n); ++i)
                {
                    auto bit = v.bit(i);
                    ctx.a_L.push_back(bit);
                    if (bit == 0)
                    {
                        ctx.a_R.push_back(minus_one);
                        A -= ctx.crs.h.at(j * ctx.crs.n + i);
                    }
                    else
                    {
                        ctx.a_R.push_back(0);
                        A += ctx.crs.g.at(j * ctx.crs.n + i);
                    }
                }
                for (u32 i = ctx.a_L.size(); i < (j + 1) * ctx.crs.n; ++i)
                {
                    ctx.a_L.push_back(0);
                    ctx.a_R.push_back(minus_one);

                    A -= ctx.crs.h.at(i);
                }
            }
            ctx.s_L.reserve(ctx.crs.n);
            ctx.s_R.reserve(ctx.crs.n);
            for (u32 i = 0; i < ctx.crs.n * ctx.crs.m; ++i)
            {
                ctx.s_L.push_back(BN::rand());
                ctx.s_R.push_back(BN::rand());
            }
            ctx.rho = BN::rand();

            auto S = G::get_gen() * ctx.rho + ctx.crs.g * ctx.s_L + ctx.crs.h * ctx.s_R;

            Serializer(buffer) << A << S;

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            Deserializer deserializer(buffer);
            deserializer >> ctx.y >> ctx.z;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            ctx.tau_1 = BN::rand();
            ctx.tau_2 = BN::rand();

            ctx.l_0 = ctx.a_L;
            for (u32 i = 0; i < ctx.crs.n * ctx.crs.m; ++i) ctx.l_0[i] -= ctx.z;
            ctx.l_0 %= G::order();

            ctx.l_1 = ctx.s_L;

            ctx.r_0 = ctx.a_R;
            ctx.r_1 = ctx.s_R;
            for (u32 i = 0; i < ctx.crs.n * ctx.crs.m; ++i) ctx.r_0[i] += ctx.z;
            auto f_y = BN(1);
            for (u32 i = 0; i < ctx.crs.n * ctx.crs.m; ++i)
            {
                ctx.r_0[i] = (ctx.r_0[i] * f_y) % G::order();
                ctx.r_1[i] = (ctx.r_1[i] * f_y) % G::order();
                if (i != ctx.crs.n * ctx.crs.m - 1)
                {
                    f_y = (f_y * ctx.y) % G::order();
                }
            }

            auto zz = ctx.z;
            for (u32 j = 0; j < ctx.crs.m; ++j)
            {
                zz = (zz * ctx.z) % G::order();
                auto zz_times_2_to_n = zz;
                for (u32 i = 0; i < ctx.crs.n; ++i)
                {
                    ctx.r_0[j * ctx.crs.n + i] += zz_times_2_to_n;
                    zz_times_2_to_n = (zz_times_2_to_n << 1) % G::order();
                }
            }
            ctx.r_0 %= G::order();

            auto t_1 = (ctx.l_0 * ctx.r_1 + ctx.l_1 * ctx.r_0) % G::order();
            auto t_2 = (ctx.l_1 * ctx.r_1) % G::order();

            auto T_1 = ctx.crs.crs_com.H[0] * t_1 + G::get_gen() * ctx.tau_1;
            auto T_2 = ctx.crs.crs_com.H[0] * t_2 + G::get_gen() * ctx.tau_2;

            Serializer(buffer) << T_1 << T_2;

            ctx.state = 2;
            return true;
        }
        else if (ctx.state == 2)
        {
            BN x;
            Deserializer deserializer(buffer);
            deserializer >> x;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            auto l = ctx.l_0 + ctx.l_1 * x;
            auto r = ctx.r_0 + ctx.r_1 * x;
            l %= G::order();
            r %= G::order();

            auto t_hat = (l * r) % G::order();

            auto tau_x = ctx.tau_2 * ((x * x) % G::order()) + ctx.tau_1 * x;
            auto zz = ctx.z;
            for (u32 i = 0; i < ctx.crs.m; ++i)
            {
                zz = (zz * ctx.z) % G::order();
                tau_x += zz * std::get<1>(ctx.openings.at(i));
            }
            tau_x %= G::order();

            auto mu = (ctx.alpha + ctx.rho * x) % G::order();

            #if USE_BULLETPROOFS_IPA == 0 && USE_QESA_IPA == 0
            {
                Serializer(buffer) << tau_x << mu << t_hat << l << r;

                ctx.state = 3;
                return false;
            }
            #else
            {

                auto y_inv = ctx.y.mod_inverse(G::order());
                auto f = y_inv;
                for (u32 i = 1; i < ctx.crs.n * ctx.crs.m; ++i)
                {
                    ctx.crs.h[i] *= f;
                    if (i != ctx.crs.n * ctx.crs.m - 1) f = (f * y_inv) % G::order();
                }

                auto P = ctx.crs.g * l + ctx.crs.h * r + G::get_gen() * mu;

                Serializer(buffer) << tau_x << mu << t_hat << P;

                ctx.crs.n = round_to_next_power_of_two(ctx.crs.n * ctx.crs.m);
                ctx.crs.m = 1;
                while (ctx.crs.g.size() < ctx.crs.n)
                {
                    ctx.crs.g.push_back(G::get_infty());
                    ctx.crs.h.push_back(G::get_infty());
                    l.emplace_back(0);
                    r.emplace_back(0);
                }

                #if USE_BULLETPROOFS_IPA
                bulletproofs::ipa::begin(ctx.ipa_ctx, ctx.crs, l, r);
                #else // QESA IPA
                qesa::CRS qesa_crs;
                qesa_crs.g_p = ctx.crs.g;
                qesa_crs.g_pp = ctx.crs.h;
                qesa_crs.Q = ctx.crs.u;
                qesa_crs.n = ctx.crs.n;
                qesa::ipa_no_zk::begin(ctx.ipa_ctx, qesa_crs, l, r, t_hat);
                #endif

                ctx.state = 3;
                return true;
            }
            #endif
        }

        #if USE_BULLETPROOFS_IPA
        else if (ctx.state == 3)
        {
            if (!bulletproofs::ipa::step_prover(ctx.ipa_ctx, buffer))
            {
                ctx.state = 4;
                return false;
            }
            return true;
        }
        #endif

        #if USE_QESA_IPA
        else if (ctx.state == 3)
        {
            if (!qesa::ipa_no_zk::step_prover(ctx.ipa_ctx, buffer))
            {
                ctx.state = 4;
                return false;
            }
            return true;
        }
        #endif

        throw std::runtime_error("reached unknown state");
    }

    bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer)
    {
        // std::cout << "bulletproofs::step_verifier " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            Deserializer deserializer(buffer);
            deserializer >> ctx.A >> ctx.S;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            ctx.y = BN::rand();
            ctx.z = BN::rand();

            Serializer(buffer) << ctx.y << ctx.z;

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            Deserializer deserializer(buffer);
            deserializer >> ctx.T_1 >> ctx.T_2;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            ctx.x = BN::rand();
            Serializer(buffer) << ctx.x;

            ctx.state = 2;
            return true;
        }
        #if USE_BULLETPROOFS_IPA == 0 && USE_QESA_IPA == 0
        else if (ctx.state == 2)
        {
            ctx.state = 3;

            BN tau_x, mu, t_hat;
            math::vector<BN> l, r;
            Deserializer deserializer(buffer);
            deserializer >> tau_x >> mu >> t_hat >> l >> r;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            {
                auto d = delta(ctx.y, ctx.z, ctx.crs.n, ctx.crs.m);

                auto lhs = ctx.crs.crs_com.H[0] * t_hat + G::get_gen() * tau_x;
                auto zz = (ctx.z * ctx.z) % G::order();
                auto rhs = ctx.crs.crs_com.H[0] * d + ctx.T_1 * ctx.x + ctx.T_2 * ((ctx.x * ctx.x) % G::order());
                for (const auto& V : ctx.commitments)
                {
                    rhs += V * zz;
                    zz = (zz * ctx.z) % G::order();
                }

                if (lhs != rhs)
                {
                    return false;
                }
            }

            // verify bulletproofs eq 66/67
            {
                auto y_inv = ctx.y.mod_inverse(G::order());
                auto f = y_inv;
                for (u32 i = 1; i < ctx.crs.n * ctx.crs.m; ++i)
                {
                    ctx.crs.h[i] *= f;
                    if (i != ctx.crs.n * ctx.crs.m - 1) f = (f * y_inv) % G::order();
                }

                auto P = ctx.A + ctx.S * ctx.x;

                for (u32 i = 0; i < ctx.crs.n * ctx.crs.m; ++i)
                {
                    P -= ctx.crs.g.at(i) * ctx.z;
                }

                auto z_times_y_to_n = ctx.z;
                for (u32 i = 0; i < ctx.crs.n * ctx.crs.m; ++i)
                {
                    P += ctx.crs.h.at(i) * z_times_y_to_n;
                    if (i != ctx.crs.n * ctx.crs.m - 1)
                    {
                        z_times_y_to_n = (z_times_y_to_n * ctx.y) % G::order();
                    }
                }

                auto zz = ctx.z;
                for (u32 j = 0; j < ctx.crs.m; ++j)
                {
                    zz = (zz * ctx.z) % G::order();
                    auto zz_times_2_to_n = zz;
                    for (u32 i = 0; i < ctx.crs.n; ++i)
                    {
                        P += ctx.crs.h.at(j * ctx.crs.n + i) * zz_times_2_to_n;
                        if (i != ctx.crs.n - 1)
                        {
                            zz_times_2_to_n = (zz_times_2_to_n << 1) % G::order();
                        }
                    }
                }

                if (P != G::get_gen() * mu + ctx.crs.g * l + ctx.crs.h * r)
                {
                    return false;
                }
            }

            // verify bulletproofs eq 68
            {
                if (t_hat != (l * r) % G::order())
                {
                    return false;
                }
            }

            ctx.result = true;

            return false;
        }

        #else

        else if (ctx.state == 2)
        {

            BN tau_x, mu, t_hat;
            G P;
            Deserializer deserializer(buffer);
            deserializer >> tau_x >> mu >> t_hat >> P;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            {
                auto d = delta(ctx.y, ctx.z, ctx.crs.n, ctx.crs.m);

                auto lhs = ctx.crs.crs_com.H[0] * t_hat + G::get_gen() * tau_x;
                auto zz = (ctx.z * ctx.z) % G::order();
                auto rhs = ctx.crs.crs_com.H[0] * d + ctx.T_1 * ctx.x + ctx.T_2 * ((ctx.x * ctx.x) % G::order());
                for (const auto& V : ctx.commitments)
                {
                    rhs += V * zz;
                    zz = (zz * ctx.z) % G::order();
                }

                ctx.result = (lhs == rhs);
            }

            // start IPA
            auto y_inv = ctx.y.mod_inverse(G::order());
            auto f = y_inv;
            for (u32 i = 1; i < ctx.crs.n * ctx.crs.m; ++i)
            {
                ctx.crs.h[i] *= f;
                if (i != ctx.crs.n * ctx.crs.m - 1) f = (f * y_inv) % G::order();
            }

            // verify bulletproofs eq 66/67
            {
                auto P_check = ctx.A + ctx.S * ctx.x;

                for (u32 i = 0; i < ctx.crs.n * ctx.crs.m; ++i)
                {
                    P_check -= ctx.crs.g.at(i) * ctx.z;
                }

                auto z_times_y_to_n = ctx.z;
                for (u32 i = 0; i < ctx.crs.n * ctx.crs.m; ++i)
                {
                    P_check += ctx.crs.h.at(i) * z_times_y_to_n;
                    if (i != ctx.crs.n * ctx.crs.m - 1)
                    {
                        z_times_y_to_n = (z_times_y_to_n * ctx.y) % G::order();
                    }
                }

                auto zz = ctx.z;
                for (u32 j = 0; j < ctx.crs.m; ++j)
                {
                    zz = (zz * ctx.z) % G::order();
                    auto zz_times_2_to_n = zz;
                    for (u32 i = 0; i < ctx.crs.n; ++i)
                    {
                        P_check += ctx.crs.h.at(j * ctx.crs.n + i) * zz_times_2_to_n;
                        if (i != ctx.crs.n - 1)
                        {
                            zz_times_2_to_n = (zz_times_2_to_n << 1) % G::order();
                        }
                    }
                }

                ctx.result &= (P == P_check);
            }

            P -= G::get_gen() * mu;

            ctx.crs.n = round_to_next_power_of_two(ctx.crs.n * ctx.crs.m);
            ctx.crs.m = 1;
            while (ctx.crs.g.size() < ctx.crs.n)
            {
                ctx.crs.g.push_back(G::get_infty());
                ctx.crs.h.push_back(G::get_infty());
            }

            #if USE_BULLETPROOFS_IPA
            bulletproofs::ipa::begin(ctx.ipa_ctx, ctx.crs, P, t_hat);
            #endif

            #if USE_QESA_IPA
            qesa::CRS qesa_crs;
            qesa_crs.g_p = ctx.crs.g;
            qesa_crs.g_pp = ctx.crs.h;
            qesa_crs.Q = ctx.crs.u;
            qesa_crs.n = ctx.crs.n;

            P += qesa_crs.Q * t_hat;

            qesa::ipa_no_zk::begin(ctx.ipa_ctx, qesa_crs, P, t_hat);
            #endif

            ctx.state = 3;
        }
        #endif

        #if USE_BULLETPROOFS_IPA
        // no else if!
        if (ctx.state == 3)
        {
            if (!bulletproofs::ipa::step_verifier(ctx.ipa_ctx, buffer))
            {
                ctx.result &= bulletproofs::ipa::get_result(ctx.ipa_ctx);

                ctx.state = 4;
                return false;
            }
            return true;
        }
        #endif

        #if USE_QESA_IPA
        // no else if!
        if (ctx.state == 3)
        {
            if (!qesa::ipa_no_zk::step_verifier(ctx.ipa_ctx, buffer))
            {
                ctx.result &= qesa::ipa_no_zk::get_result(ctx.ipa_ctx);

                ctx.state = 4;
                return false;
            }
            return true;
        }
        #endif

        throw std::runtime_error("reached unknown state");
    }

    bool get_result(VerifierContext& ctx)
    {
        return ctx.result;
    }
}
