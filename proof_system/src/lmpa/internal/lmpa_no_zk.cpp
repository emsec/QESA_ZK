#include "lmpa/internal/lmpa_no_zk.h"
#include <cmath>

namespace lmpa::no_zk
{

    void begin(ProverContext& ctx, const Matrix<G>& A, const math::vector<BN>& w)
    {
        ctx.A = A;
        ctx.w = w;
        ctx.first_iteration = true;
        ctx.n = round_to_next_power_of_two(ctx.A.cols());
    }

    void begin(VerifierContext& ctx, const Matrix<G>& A, const math::vector<G>& t)
    {
        ctx.A = A;
        ctx.t = t;
        ctx.n = round_to_next_power_of_two(ctx.A.cols());
    }

    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer)
    {
        // std::cout << "lmpa::no_zk::step_prover " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            if (!ctx.first_iteration)
            {
                BN xi;

                Deserializer deserializer(buffer);
                deserializer >> xi;
                if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

                for (u32 row = 0; row < ctx.A.rows(); ++row)
                {
                    for (u32 col = 0; col < ctx.n / 2 && col + ctx.n / 2 < ctx.A.cols(); ++col)
                    {
                        ctx.A(row, col) += ctx.A(row, col + ctx.n / 2) * xi;
                    }
                }

                for (u32 i = 0; i < ctx.n / 2; ++i)
                {
                    ctx.w[i] *= xi;
                    if (i + ctx.n / 2 < ctx.A.cols())
                    {
                        ctx.w[i] += ctx.w.at(i + ctx.n / 2);
                    }
                    ctx.w[i] %= G::order();
                }

                ctx.n /= 2;
            }
            ctx.first_iteration = false;

            if (ctx.n == 1)
            {
                Serializer(buffer) << ctx.w.at(0);
                ctx.state = 1;
                return false;
            }


            //u_{-1} = A_0 w_1
            //u_{+1} = A_1 w_0

            ctx.u_minus_one.clear();
            ctx.u_plus_one.clear();
            for (u32 row = 0; row < ctx.A.rows(); ++row)
            {
                {
                    auto[begin, end] = ctx.A.get_row_part(row, 0, ctx.n / 2);
                    auto w_it = ctx.w.begin() + ctx.n / 2;
                    G tmp = 0;
                    for (auto it = begin; it != end && w_it != ctx.w.end(); ++it)
                    {
                        tmp += (*it) * (*w_it);
                        w_it++;
                    }
                    ctx.u_minus_one.push_back(tmp);
                }
                {
                    u32 end_column = std::min(ctx.n, ctx.A.cols());
                    auto[begin, end] = ctx.A.get_row_part(row, ctx.n / 2, end_column);
                    auto w_it = ctx.w.begin();
                    G tmp = 0;
                    for (auto it = begin; it != end && w_it != ctx.w.end(); ++it)
                    {
                        tmp += (*it) * (*w_it);
                        w_it++;
                    }
                    ctx.u_plus_one.push_back(tmp);
                }
            }

            Serializer(buffer) << ctx.u_minus_one << ctx.u_plus_one;
            return true;
        }

        throw std::runtime_error("reached unknown state");
    }

    bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer)
    {
        // std::cout << "lmpa::no_zk::step_verifier " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            if (ctx.n == 1)
            {
                BN w;

                Deserializer deserializer(buffer);
                deserializer >> w;
                if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

                ctx.result = true;
                for (u32 row = 0; row < ctx.A.rows(); ++row)
                {
                    if (ctx.A(row, 0) * w != ctx.t.at(row))
                    {
                        ctx.result = false;
                        break;
                    }
                }

                ctx.state = 1;
                return false;
            }

            math::vector<G> u_minus_one;
            math::vector<G> u_plus_one;

            Deserializer deserializer(buffer);
            deserializer >> u_minus_one >> u_plus_one;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            #if USE_SMALL_EXPONENTS
            auto xi = BN::rand(SMALL_EXP_SIZE, true);
            #else
            auto xi = BN::rand();
            #endif

            Serializer(buffer) << xi;

            for (u32 row = 0; row < ctx.A.rows(); ++row)
            {
                for (u32 col = 0; col < ctx.n / 2 && col + ctx.n / 2 < ctx.A.cols(); ++col)
                {
                    ctx.A(row, col) += ctx.A(row, col + ctx.n / 2) * xi;
                }
            }

            ctx.t = u_plus_one * ((xi * xi) % G::order()) + ctx.t * xi + u_minus_one;

            ctx.n /= 2;
            return true;
        }

        throw std::runtime_error("reached unknown state");
    }

    bool get_result(VerifierContext& ctx)
    {
        return ctx.result;
    }
}
