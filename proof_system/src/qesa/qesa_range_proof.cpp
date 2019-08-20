#include "qesa/qesa_range_proof.h"

namespace qesa::range_proof
{
    G commit(const CRS& crs, const BN& m, const BN& r)
    {
        return qesa::copy::commit(crs, {1}, {m}, r);
    }


    void begin(ProverContext& ctx, u32 range, const std::vector<std::tuple<BN, BN>>& openings)
    {
        if (ctx.crs.n < 2 + range * openings.size())
        {
            throw std::runtime_error("QESA CRS is too small");
        }
        ctx.range = range;
        ctx.openings = openings;
    }

    void begin(VerifierContext& ctx, u32 range, const std::vector<G>& commitments)
    {
        if (ctx.crs.n < 2 + range * commitments.size())
        {
            throw std::runtime_error("QESA CRS is too small");
        }
        ctx.range = range;
        ctx.commitments = commitments;
    }


    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer)
    {
        // std::cout << "qesa::range_proof::step_prover " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            ctx.r[0] = BN::rand();
            ctx.r[1] = BN::rand();

            ctx.a = ctx.crs.g_p.at(1) * ctx.r[0] + ctx.crs.g_p.back() * ctx.r[1];

            Serializer(buffer) << ctx.a;

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            math::vector<BN> x;

            Deserializer deserializer(buffer);
            deserializer >> x;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            BN z[2] = {ctx.r[0], ctx.r[1]};

            for (u32 i = 0; i < ctx.openings.size(); ++i)
            {
                const auto& [w, r] = ctx.openings.at(i);
                z[0] += x.at(i) * w;
                z[1] += x.at(i) * r;
            }

            z[0] %= G::order();
            z[1] %= G::order();

            Serializer(buffer) << z[0] << z[1];

            ///////////////////////////////////////////////
            // prepare qesa::copy

            // compute mapping
            std::vector<qesa::copy::CommitmentCRS> com_crs_set;
            for (u32 i = 0; i < ctx.openings.size(); ++i)
            {
                com_crs_set.push_back({1});
            }
            auto mapping = qesa::copy::compute_mapping(ctx.crs, 1, com_crs_set);

            // generate matrices and witness
            math::vector<BN> witness(ctx.crs.n - 2, BN(0));
            witness[0] = 1;

            std::vector<qesa::SparseMatrix> matrices;

            u32 index = mapping.last_index + 1;
            for (u32 opening = 0; opening < ctx.openings.size(); ++opening)
            {
                auto value = std::get<0>(ctx.openings.at(opening));

                // matrices and witness for "input is a bit"
                for (u32 i = 0; i < ctx.range - 1; ++i)
                {
                    qesa::SparseMatrix bit_matrix;
                    math::SparseVector<BN> a;
                    math::SparseVector<BN> b;
                    a.set(index + i, 1);
                    b.set(0, -1);
                    b.set(index + i, 1);
                    bit_matrix.vectors.emplace_back(a, b);
                    matrices.push_back(bit_matrix);

                    witness[index + i] = value.bit(i + 1);
                }

                // matrix for "(value - sum of all bits except lsb) is a bit"
                {
                    qesa::SparseMatrix sum_matrix;
                    math::SparseVector<BN> a;
                    math::SparseVector<BN> b;
                    a.set(mapping.msg_id_to_msg_position.at(opening), 1);
                    b.set(0, -1);
                    b.set(mapping.msg_id_to_msg_position.at(opening), 1);
                    BN factor = 2;
                    for (u32 i = 0; i < ctx.range - 1; ++i)
                    {
                        a.set(index + i, -factor);
                        b.set(index + i, -factor);
                        factor <<= 1;
                    }
                    sum_matrix.vectors.emplace_back(a, b);
                    matrices.push_back(sum_matrix);
                }

                index += ctx.range - 1;
            }

            std::vector<std::tuple<std::vector<BN>, BN>> qesa_copy_openings;
            for (const auto& [w, r] : ctx.openings)
            {
                qesa_copy_openings.emplace_back(std::make_tuple(std::vector<BN>(1, w), r));
            }
            qesa::copy::begin(ctx.qesa_copy_ctx, matrices, mapping, witness, qesa_copy_openings);

            ctx.state = 2;
            return true;
        }
        else if (ctx.state == 2)
        {
            if (!qesa::copy::step_prover(ctx.qesa_copy_ctx, buffer))
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
        // std::cout << "qesa::range_proof::step_verifier " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            G a;
            Deserializer deserializer(buffer);
            deserializer >> a;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            math::vector<BN> x;
            for (u32 i = 0; i < ctx.commitments.size(); ++i)
            {
                #if USE_SMALL_EXPONENTS
                x.push_back(BN::rand(SMALL_EXP_SIZE, true));
                #else
                x.push_back(BN::rand());
                #endif
            }

            Serializer(buffer) << x;

            ctx.rhs = a;
            for (u32 i = 0; i < ctx.commitments.size(); ++i)
            {
                ctx.rhs += x.at(i) * ctx.commitments.at(i);
            }

            ctx.state = 1;
            return true;
        }
        else if (ctx.state == 1)
        {
            BN z[2];
            Deserializer deserializer(buffer);
            deserializer >> z[0] >> z[1];
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            ctx.result = (ctx.crs.g_p.at(1) * z[0] + ctx.crs.g_p.back() * z[1] == ctx.rhs);

            ///////////////////////////////////////////////
            // prepare qesa::copy

            // compute mapping
            std::vector<qesa::copy::CommitmentCRS> com_crs_set;
            for (u32 i = 0; i < ctx.commitments.size(); ++i)
            {
                com_crs_set.push_back({1});
            }
            auto mapping = qesa::copy::compute_mapping(ctx.crs, 1, com_crs_set);

            // generate matrices and witness
            std::vector<qesa::SparseMatrix> matrices;

            u32 index = mapping.last_index + 1;
            for (u32 commitment = 0; commitment < ctx.commitments.size(); ++commitment)
            {
                // matrices and witness for "input is a bit"
                for (u32 i = 0; i < ctx.range - 1; ++i)
                {
                    qesa::SparseMatrix bit_matrix;
                    math::SparseVector<BN> a;
                    math::SparseVector<BN> b;
                    a.set(index + i, 1);
                    b.set(0, -1);
                    b.set(index + i, 1);
                    bit_matrix.vectors.emplace_back(a, b);
                    matrices.push_back(bit_matrix);
                }

                // matrix for "(value - sum of all bits except lsb) is a bit"
                {
                    qesa::SparseMatrix sum_matrix;
                    math::SparseVector<BN> a;
                    math::SparseVector<BN> b;
                    a.set(mapping.msg_id_to_msg_position.at(commitment), 1);
                    b.set(0, -1);
                    b.set(mapping.msg_id_to_msg_position.at(commitment), 1);
                    BN factor = 2;
                    for (u32 i = 0; i < ctx.range - 1; ++i)
                    {
                        a.set(index + i, -factor);
                        b.set(index + i, -factor);
                        factor <<= 1;
                    }
                    sum_matrix.vectors.emplace_back(a, b);
                    matrices.push_back(sum_matrix);
                }

                index += ctx.range - 1;
            }

            qesa::copy::begin(ctx.qesa_copy_ctx, matrices, mapping, ctx.commitments);

            ctx.state = 2;
            return true;
        }
        else if (ctx.state == 2)
        {
            if (!qesa::copy::step_verifier(ctx.qesa_copy_ctx, buffer))
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
        return ctx.result && qesa::copy::get_result(ctx.qesa_copy_ctx);
    }

}
