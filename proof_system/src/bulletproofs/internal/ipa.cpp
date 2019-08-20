#include "bulletproofs/internal/ipa.h"

#if USE_BULLETPROOFS_IPA

namespace bulletproofs::ipa
{
    namespace
    {
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
    void begin(ProverContext& ctx, const CRS& crs, const math::vector<BN>& a, const math::vector<BN>& b)
    {
        ctx.crs = crs;
        ctx.a = a;
        ctx.b = b;

        ctx.first_iteration = true;
    }

    void begin(VerifierContext& ctx, const CRS& crs, const G& P, const BN& c)
    {
        ctx.crs = crs;
        ctx.P = P;
        ctx.c = c;
    }

    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer)
    {
        // std::cout << "bulletproofs::ipa::step_prover " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            BN xi;
            Deserializer deserializer(buffer);
            deserializer >> xi;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            ctx.crs.u *= xi;

            ctx.state = 1;
        }
        // no else if!
        if (ctx.state == 1)
        {
            if (!ctx.first_iteration)
            {
                BN x;
                Deserializer deserializer(buffer);
                deserializer >> x;
                if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

                auto x_inv = x.mod_inverse(G::order());

                mix(ctx.crs.g, x, x_inv);
                mix(ctx.crs.h, x_inv, x);

                mix(ctx.a, x_inv, x);
                ctx.a %= G::order();

                mix(ctx.b, x, x_inv);
                ctx.b %= G::order();
            }
            ctx.first_iteration = false;

            if (ctx.crs.n == 1)
            {
                Serializer(buffer) << ctx.a << ctx.b;

                ctx.state = 2;
                return false;
            }

            ctx.crs.n /= 2;

            BN c_L = 0;
            BN c_R = 0;

            for (u32 i = 0; i < ctx.crs.n; ++i)
            {
                c_L += ctx.a.at(ctx.crs.n + i) * ctx.b.at(i);
                c_R += ctx.a.at(i) * ctx.b.at(ctx.crs.n + i);
            }
            c_L %= G::order();
            c_R %= G::order();

            auto L = ctx.crs.u * c_L;
            auto R = ctx.crs.u * c_R;

            for (u32 i = 0; i < ctx.crs.n; ++i)
            {
                L += ctx.crs.g.at(i) * ctx.a.at(ctx.crs.n + i) + ctx.crs.h.at(ctx.crs.n + i) * ctx.b.at(i);
                R += ctx.crs.g.at(ctx.crs.n + i) * ctx.a.at(i) + ctx.crs.h.at(i) * ctx.b.at(ctx.crs.n + i);
            }

            Serializer(buffer) << L << R;

            return true;
        }
        throw std::runtime_error("reached unknown state");
    }

    bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer)
    {
        // std::cout << "bulletproofs::ipa::step_verifier " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            auto xi = BN::rand();
            Serializer(buffer) << xi;

            ctx.crs.u *= xi;
            ctx.P += ctx.crs.u * ctx.c;

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            if (ctx.crs.n == 1)
            {
                Deserializer deserializer(buffer);
                math::vector<BN> a, b;
                deserializer >> a >> b;
                if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

                auto c = (a * b) % G::order();

                ctx.result = (ctx.P == ctx.crs.g * a + ctx.crs.h * b + ctx.crs.u * c);

                ctx.state = 2;
                return false;
            }

            ctx.crs.n /= 2;

            Deserializer deserializer(buffer);
            G L, R;
            deserializer >> L >> R;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            BN x = BN::rand();

            auto x_inv = x.mod_inverse(G::order());

            mix(ctx.crs.g, x, x_inv); // = ctx.crs.g_R * x_inv + ctx.crs.g_L * x;
            mix(ctx.crs.h, x_inv, x); // = ctx.crs.h_R * x + ctx.crs.h_L * x_inv;
            ctx.P = L * ((x * x) % G::order()) + ctx.P + R * ((x_inv * x_inv) % G::order());

            Serializer(buffer) << x;
            return true;
        }
        throw std::runtime_error("reached unknown state");
    }

    bool get_result(VerifierContext& ctx)
    {
        return ctx.result;
    }
}

#endif
