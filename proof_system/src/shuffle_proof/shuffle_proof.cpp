#include "shuffle_proof/shuffle_proof.h"

namespace shuffle_proof
{
    CRS gen_CRS(u32 shuffle_size)
    {
        CRS crs;
        crs.qesa_crs = qesa::gen_CRS(shuffle_size * 2);
        crs.shuffle_size = shuffle_size;
        return crs;
    }

    void begin(ProverContext& ctx,
               const std::vector<el_gamal::Ciphertext>& c_old,
               const std::vector<el_gamal::Ciphertext>& c_new,
               const el_gamal::PublicKey& pk,
               const std::vector<u32>& permutation,
               const math::vector<BN>& rerandomization)
    {
        if (c_old.size() != ctx.crs.shuffle_size ||
                c_new.size() != ctx.crs.shuffle_size ||
                permutation.size() != ctx.crs.shuffle_size ||
                rerandomization.size() != ctx.crs.shuffle_size)
        {
            throw std::runtime_error("input has invalid size");
        }

        ctx.c_old = c_old;
        ctx.c_new = c_new;
        ctx.pk = pk;
        ctx.pi = permutation;
        ctx.rerandomization = rerandomization;
    }

    void begin(VerifierContext& ctx,
               const std::vector<el_gamal::Ciphertext>& c_old,
               const std::vector<el_gamal::Ciphertext>& c_new,
               const el_gamal::PublicKey& pk)
    {
        if (c_old.size() != ctx.crs.shuffle_size ||
                c_new.size() != ctx.crs.shuffle_size)
        {
            throw std::runtime_error("input has invalid size");
        }

        ctx.c_old = c_old;
        ctx.c_new = c_new;
        ctx.pk = pk;
    }

    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer)
    {
        // std::cout << "shuffle_proof::step_prover " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            // compute commitment to pi
            ctx.r_pi = BN::rand();
            ctx.com_pi = ctx.crs.qesa_crs.g_p.back() * ctx.r_pi;
            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                ctx.com_pi += ctx.crs.qesa_crs.g_p.at(i + 1) * BN(ctx.pi.at(i));
            }

            Serializer(buffer) << ctx.com_pi;

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            Deserializer deserializer(buffer);
            deserializer >> ctx.x;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            // compute commitment to y = pi(x)
            ctx.r_y = BN::rand();
            ctx.com_y = ctx.crs.qesa_crs.g_p.back() * ctx.r_y;
            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                ctx.y.push_back(ctx.x.at(ctx.pi.at(i)));
                ctx.com_y += ctx.crs.qesa_crs.g_p.at(i + 1) * ctx.y.at(i);
            }

            Serializer(buffer) << ctx.com_y;

            ctx.state = 2;
            return true;
        }
        else if (ctx.state == 2)
        {
            Deserializer deserializer(buffer);
            deserializer >> ctx.xi >> ctx.z;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            // prepare modified QESA_copy
            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                ctx.d.push_back((ctx.xi * BN(ctx.pi.at(i)) + ctx.y.at(i)) % G::order());
            }

            ctx.qesa_inner_ctx.v_p = math::vector<BN>(ctx.crs.qesa_crs.n - 2, BN(0));
            ctx.qesa_inner_ctx.v_p[0] = 1;

            u32 last_result_index = ctx.crs.shuffle_size;

            // compute intermediate product values, skip last value as it is the final result
            for (u32 i = 0; i < ctx.crs.shuffle_size - 1; ++i)
            {
                BN witness_entry;
                if (i == 0)
                {
                    witness_entry = ((ctx.d.at(0) - ctx.z) * (ctx.d.at(1) - ctx.z)) % G::order();
                    ++i;
                }
                else
                {
                    witness_entry = (ctx.qesa_inner_ctx.v_p.at(last_result_index) * (ctx.d.at(i) - ctx.z)) % G::order();
                }
                last_result_index++;

                ctx.qesa_inner_ctx.v_p[last_result_index] = witness_entry;
            }

            ctx.qesa_inner_ctx.v_p.push_back(BN::rand());
            ctx.qesa_inner_ctx.v_p.push_back(BN::rand());

            auto c_p = ctx.crs.qesa_crs.g_p * ctx.qesa_inner_ctx.v_p;

            Serializer(buffer) << c_p;

            ctx.state = 3;
            return true;
        }
        else if (ctx.state == 3)
        {
            BN alpha;
            Deserializer deserializer(buffer);
            deserializer >> alpha;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                ctx.qesa_inner_ctx.v_p[i + 1] = (alpha * ctx.d.at(i)) % G::order();
            }
            ctx.qesa_inner_ctx.v_p[ctx.crs.qesa_crs.n - 1] = (ctx.qesa_inner_ctx.v_p[ctx.crs.qesa_crs.n - 1] + alpha * (ctx.xi * ctx.r_pi + ctx.r_y)) % G::order();

            BN rhs = 1;
            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                rhs = (rhs * (ctx.xi * BN(i) + ctx.x.at(i) - ctx.z)) % G::order();
            }

            auto alpha_inv = alpha.mod_inverse(G::order());

            u32 last_result_index = ctx.crs.shuffle_size;
            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                qesa::SparseMatrix mat;
                math::SparseVector<BN> a;
                math::SparseVector<BN> b;
                if (i == 0)
                {
                    a.set(0, -ctx.z);
                    a.set(1, alpha_inv);
                    b.set(0, -ctx.z);
                    b.set(2, alpha_inv);
                    ++i; // indices 0 and 1 are already processed
                }
                else
                {
                    a.set(last_result_index, 1);
                    b.set(0, -ctx.z);
                    b.set(i + 1, alpha_inv);
                }
                mat.vectors.emplace_back(a, b);

                last_result_index++;

                a.clear();
                b.clear();
                if (i < ctx.crs.shuffle_size - 1)
                {
                    a.set(0, -1);
                    b.set(last_result_index, 1);
                }
                else
                {
                    a.set(0, -1);
                    b.set(0, rhs);
                }
                mat.vectors.emplace_back(a, b);
                ctx.qesa_inner_ctx.matrices.push_back(mat);
            }
            ctx.state = 4;
            return true;
        }
        else if (ctx.state == 4)
        {
            if (!qesa::inner::step_prover(ctx.qesa_inner_ctx, buffer))
            {
                ctx.state = 5;
            }
            return true;
        }
        else if (ctx.state == 5)
        {
            // prepare LMPA

            // compute commitment to sigma
            ctx.sigma = (ctx.x * ctx.rerandomization) % G::order();
            ctx.r_sigma = BN::rand();
            ctx.com_sigma = ctx.crs.qesa_crs.g_p.at(ctx.crs.shuffle_size + 1) * ctx.sigma + ctx.crs.qesa_crs.g_p.at(ctx.crs.qesa_crs.n - 2) * ctx.r_sigma;

            Serializer(buffer) << ctx.com_sigma;
            ctx.state = 6;
            return true;
        }
        else if (ctx.state == 6)
        {
            std::vector<BN> batch_randomness;
            Deserializer deserializer(buffer);
            deserializer >> batch_randomness;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            lmpa::Matrix<G> A(2, ctx.crs.shuffle_size + 3);
            // row 0
            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                A(0, i) = ctx.crs.qesa_crs.g_p.at(i + 1);
            }
            A(0, ctx.crs.shuffle_size) = ctx.crs.qesa_crs.g_p.at(ctx.crs.shuffle_size + 1);
            A(0, ctx.crs.shuffle_size + 1) = ctx.crs.qesa_crs.g_p.at(ctx.crs.qesa_crs.n - 2);
            A(0, ctx.crs.shuffle_size + 2) = ctx.crs.qesa_crs.g_p.at(ctx.crs.qesa_crs.n - 1);

            // row 1
            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                A(1, i) = batch_randomness.at(0) * ctx.crs.qesa_crs.g_p.at(i + 1)
                          + batch_randomness.at(2) * ctx.c_old.at(i).c_0
                          + batch_randomness.at(3) * ctx.c_old.at(i).c_1;
            }
            A(1, ctx.crs.shuffle_size) = batch_randomness.at(1) * ctx.crs.qesa_crs.g_p.at(ctx.crs.shuffle_size + 1) + batch_randomness.at(2) * G::get_gen() + batch_randomness.at(3) * ctx.pk;

            A(1, ctx.crs.shuffle_size + 1) = batch_randomness.at(1) * ctx.crs.qesa_crs.g_p.at(ctx.crs.qesa_crs.n - 2);
            A(1, ctx.crs.shuffle_size + 2) = batch_randomness.at(0) * ctx.crs.qesa_crs.g_p.at(ctx.crs.qesa_crs.n - 1);

            math::vector<BN> witness(ctx.y.begin(), ctx.y.end());
            witness.push_back(ctx.sigma);
            witness.push_back(ctx.r_sigma);
            witness.push_back(ctx.r_y);

            lmpa::simple_zk::begin(ctx.lmpa_ctx, A, witness);

            ctx.state = 7;
        }
        // no else if!
        if (ctx.state == 7)
        {
            if (!lmpa::simple_zk::step_prover(ctx.lmpa_ctx, buffer))
            {
                ctx.state = 8;
                return false;
            }
            return true;
        }
        throw std::runtime_error("reached unknown state");
    }

    bool step_verifier(VerifierContext& ctx, std::vector<u8>& buffer)
    {
        // std::cout << "shuffle_proof::step_verifier " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            Deserializer deserializer(buffer);
            deserializer >> ctx.com_pi;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                #if USE_SMALL_EXPONENTS
                ctx.x.push_back(BN::rand(SMALL_EXP_SIZE, true));
                #else
                ctx.x.push_back(BN::rand());
                #endif
            }

            Serializer(buffer) << ctx.x;

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            Deserializer deserializer(buffer);
            deserializer >> ctx.com_y;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            ctx.xi = BN::rand();
            ctx.z = BN::rand();

            Serializer(buffer) << ctx.xi << ctx.z;

            ctx.state = 2;
            return true;
        }
        else if (ctx.state == 2)
        {
            G c;
            Deserializer deserializer(buffer);
            deserializer >> c;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            BN alpha = BN::rand();

            Serializer(buffer) << alpha;

            ctx.qesa_inner_ctx.c_p = c + alpha * (ctx.xi * ctx.com_pi + ctx.com_y);

            BN rhs = 1;
            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                rhs = (rhs * (ctx.xi * BN(i) + ctx.x.at(i) - ctx.z)) % G::order();
            }

            auto alpha_inv = alpha.mod_inverse(G::order());

            u32 last_result_index = ctx.crs.shuffle_size;
            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {

                qesa::SparseMatrix mat;
                math::SparseVector<BN> a;
                math::SparseVector<BN> b;
                if (i == 0)
                {
                    a.set(0, -ctx.z);
                    a.set(1, alpha_inv);
                    b.set(0, -ctx.z);
                    b.set(2, alpha_inv);
                    ++i; // indices 0 and 1 are already processed
                }
                else
                {
                    a.set(last_result_index, 1);
                    b.set(0, -ctx.z);
                    b.set(i + 1, alpha_inv);
                }
                mat.vectors.emplace_back(a, b);

                last_result_index++;
                a.clear();
                b.clear();
                if (i < ctx.crs.shuffle_size - 1)
                {
                    a.set(0, -1);
                    b.set(last_result_index, 1);
                }
                else
                {
                    a.set(0, -1);
                    b.set(0, rhs);
                }
                mat.vectors.emplace_back(a, b);
                ctx.qesa_inner_ctx.matrices.push_back(mat);
            }

            ctx.state = 3;
            return true;
        }
        else if (ctx.state == 3)
        {
            if (!qesa::inner::step_verifier(ctx.qesa_inner_ctx, buffer))
            {
                ctx.state = 4;
            }
            return true;
        }
        else if (ctx.state == 4)
        {
            G com_sigma;
            Deserializer deserializer(buffer);
            deserializer >> com_sigma;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            std::vector<BN> batch_randomness = {BN::rand(), BN::rand(), BN::rand(), BN::rand()};

            Serializer(buffer) << batch_randomness;

            lmpa::Matrix<G> A(2, ctx.crs.shuffle_size + 3);
            // row 0
            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                A(0, i) = ctx.crs.qesa_crs.g_p.at(i + 1);
            }
            A(0, ctx.crs.shuffle_size) = ctx.crs.qesa_crs.g_p.at(ctx.crs.shuffle_size + 1);
            A(0, ctx.crs.shuffle_size + 1) = ctx.crs.qesa_crs.g_p.at(ctx.crs.qesa_crs.n - 2);
            A(0, ctx.crs.shuffle_size + 2) = ctx.crs.qesa_crs.g_p.at(ctx.crs.qesa_crs.n - 1);

            // row 1
            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                A(1, i) = batch_randomness.at(0) * ctx.crs.qesa_crs.g_p.at(i + 1)
                          + batch_randomness.at(2) * ctx.c_old.at(i).c_0
                          + batch_randomness.at(3) * ctx.c_old.at(i).c_1;
            }
            A(1, ctx.crs.shuffle_size) = batch_randomness.at(1) * ctx.crs.qesa_crs.g_p.at(ctx.crs.shuffle_size + 1) + batch_randomness.at(2) * G::get_gen() + batch_randomness.at(3) * ctx.pk;

            A(1, ctx.crs.shuffle_size + 1) = batch_randomness.at(1) * ctx.crs.qesa_crs.g_p.at(ctx.crs.qesa_crs.n - 2);
            A(1, ctx.crs.shuffle_size + 2) = batch_randomness.at(0) * ctx.crs.qesa_crs.g_p.at(ctx.crs.qesa_crs.n - 1);


            G u[2] = {0, 0};
            for (u32 i = 0; i < ctx.crs.shuffle_size; ++i)
            {
                u[0] += ctx.x.at(i) * ctx.c_new.at(i).c_0;
                u[1] += ctx.x.at(i) * ctx.c_new.at(i).c_1;
            }

            math::vector<G> t = {ctx.com_y + com_sigma, batch_randomness.at(0) * ctx.com_y + batch_randomness.at(1) * com_sigma + batch_randomness.at(2) * u[0] + batch_randomness.at(3) * u[1]};

            lmpa::simple_zk::begin(ctx.lmpa_ctx, A, t);

            ctx.state = 5;
            return true;
        }
        else if (ctx.state == 5)
        {
            if (!lmpa::simple_zk::step_verifier(ctx.lmpa_ctx, buffer))
            {
                ctx.state = 6;
                return false;
            }
            return true;
        }
        throw std::runtime_error("reached unknown state");
    }

    bool get_result(VerifierContext& ctx)
    {
        return qesa::inner::get_result(ctx.qesa_inner_ctx) && lmpa::simple_zk::get_result(ctx.lmpa_ctx);
    }
}
