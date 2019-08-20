#include "qesa/qesa_zk.h"

namespace qesa::qesa_zk
{
    void begin(ProverContext& ctx, const std::vector<SparseMatrix>& matrices, const math::vector<BN>& witness)
    {
        if (witness.at(0) != 1)
        {
            throw std::runtime_error("witness[0] must be 1");
        }
        ctx.matrices = matrices;

        ctx.v_p = witness;
        while (ctx.v_p.size() < ctx.crs.n - 2)
        {
            ctx.v_p.emplace_back(0);
        }

        ctx.v_p.push_back(BN::rand());
        ctx.v_p.push_back(BN::rand());

        ctx.state = 0;
    }

    void begin(VerifierContext& ctx, const std::vector<SparseMatrix>& matrices)
    {
        ctx.matrices = matrices;
        ctx.state = 0;
    }

    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer)
    {
        if (ctx.state == 0)
        {
            // std::cout << "engaging QESA_zk..." << std::endl;
            auto c_p = ctx.crs.g_p * ctx.v_p;
            Serializer(buffer) << c_p;

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            if (!inner::step_prover(ctx, buffer))
            {
                ctx.state = 2;
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
            deserializer >> ctx.c_p;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            inner::step_verifier(ctx, buffer);

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            if (!inner::step_verifier(ctx, buffer))
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
        return inner::get_result(ctx);
    }
}
