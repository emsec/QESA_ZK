#include "lmpa/lmpa_simple_zk.h"

namespace lmpa::simple_zk
{

    void begin(ProverContext& ctx, const Matrix<G>& A, const math::vector<BN>& w)
    {
        if (A.cols() != w.size())
        {
            throw std::runtime_error("input has invalid size");
        }
        ctx.A = A;
        ctx.w = w;
    }
    void begin(VerifierContext& ctx, const Matrix<G>& A, const math::vector<G>& t)
    {
        if (A.rows() != t.size())
        {
            throw std::runtime_error("input has invalid size");
        }
        ctx.A = A;
        ctx.t = t;
    }

    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer)
    {
        // std::cout << "lmpa::simple_zk::step_prover " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            for (u32 i = 0; i < ctx.A.cols(); ++i)
            {
                ctx.r.push_back(BN::rand());
            }

            ctx.a = ctx.A * ctx.r;

            Serializer(buffer) << ctx.a;

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            BN beta;

            Deserializer deserializer(buffer);
            deserializer >> beta;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            no_zk::begin(ctx, ctx.A, (ctx.w * beta + ctx.r) % G::order());

            ctx.state = 2;
        }
        // no else if!
        if (ctx.state == 2)
        {
            if (!no_zk::step_prover(ctx, buffer))
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
        // std::cout << "lmpa::simple_zk::step_verifier " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            math::vector<G> a;

            Deserializer deserializer(buffer);
            deserializer >> a;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            #if USE_SMALL_EXPONENTS
            auto beta = BN::rand(SMALL_EXP_SIZE, true);
            #else
            auto beta = BN::rand();
            #endif

            Serializer(buffer) << beta;

            no_zk::begin(ctx, ctx.A, ctx.t * beta + a);

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            if (!no_zk::step_verifier(ctx, buffer))
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
        return ctx.result;
    }
}
