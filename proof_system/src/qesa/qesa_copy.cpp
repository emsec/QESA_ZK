#include "qesa/qesa_copy.h"

namespace qesa::copy
{

    CommitmentCRS get_commitment_crs(const CRS& crs, const std::vector<u32>& generator_indices)
    {
        for (const auto& x : generator_indices)
        {
            if (x == 0 || x >= crs.n - 1)
            {
                throw std::runtime_error("invalid generator index");
            }
        }
        return generator_indices;
    }

    G commit(const CRS& crs_main, const CommitmentCRS& crs_com, const std::vector<BN>& msg, const BN& r)
    {
        if (msg.size() != crs_com.size())
        {
            throw std::runtime_error("message has different size than crs");
        }
        G res = crs_main.g_p.back() * r;
        for (u32 i = 0; i < msg.size(); ++i)
        {
            res += crs_main.g_p.at(crs_com.at(i)) * msg.at(i);
        }
        return res;
    }

    Mapping compute_mapping(const CRS& crs_main, u32 witness_size, const std::vector<CommitmentCRS>& com_crs_set)
    {
        Mapping res;

        res.num_crs = com_crs_set.size();

        for (const auto& crs : com_crs_set)
        {
            for (const auto& index : crs)
            {
                res.unique_key_indices.insert(index);
            }
        }

        u32 position = witness_size - 1;

        u32 msg_id = 0;
        u32 crs_id = 0;

        for (const auto& crs : com_crs_set)
        {
            for (const auto& key_id : crs)
            {
                position++;
                while (res.unique_key_indices.find(position) != res.unique_key_indices.end())
                {
                    position++;
                }

                res.key_position_for_message[msg_id] = key_id;
                res.msg_id_to_msg_position[msg_id] = position;
                res.crs_id_for_message[msg_id] = crs_id;
                res.messages_for_key[key_id].push_back(msg_id);

                msg_id++;
            }
            crs_id++;
        }

        res.last_index = position;

        if (position >= crs_main.n - 2)
        {
            throw std::runtime_error("n is of insufficient size for qesa copy");
        }

        return res;
    }


    void begin(ProverContext& ctx, const std::vector<SparseMatrix>& matrices,
               const Mapping& map, const math::vector<BN>& witness,
               const std::vector<std::tuple<std::vector<BN>, BN>>& openings)
    {
        ctx.matrices = matrices;
        ctx.openings = openings;
        ctx.mapping = map;

        if (map.num_crs != openings.size())
        {
            throw std::runtime_error("number of commitments does not fit the number of commitment CRS");
        }
        if (witness.at(0) != 1)
        {
            throw std::runtime_error("witness[0] must be 1");
        }

        ctx.v_p = witness;
        while (ctx.v_p.size() < ctx.crs.n - 2)
        {
            ctx.v_p.emplace_back(0);
        }

        ctx.v_p.push_back(BN::rand());
        ctx.v_p.push_back(BN::rand());

        u32 msg_id = 0;
        for (const auto& [message, r] : openings)
        {
            UNUSED(r);
            for (const auto& m : message)
            {
                ctx.v_p[map.msg_id_to_msg_position.at(msg_id)] = m;
                msg_id++;
            }
        }

        ctx.state = 0;
    }

    void begin(VerifierContext& ctx, const std::vector<SparseMatrix>& matrices,
               const Mapping& map, const std::vector<G>& commitments)
    {
        ctx.matrices = matrices;
        ctx.mapping = map;
        ctx.commitments = commitments;
        ctx.state = 0;

        if (map.num_crs != commitments.size())
        {
            throw std::runtime_error("number of commitments does not fit the number of commitment CRS");
        }
    }

    bool step_prover(ProverContext& ctx, std::vector<u8>& buffer)
    {
        // std::cout << "qesa_copy::step_prover " << ctx.state << std::endl;
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
            std::vector<BN> alpha;
            Deserializer deserializer(buffer);
            deserializer >> alpha;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            // both sides adjust their states

            {
                u32 msg_id = 0;
                for (u32 i = 0; i < ctx.openings.size(); ++i)
                {
                    auto& [msg, r] = ctx.openings.at(i);

                    ctx.v_p[ctx.crs.n - 1] += r * alpha.at(i);

                    for (u32 j = 0; j < msg.size(); ++j)
                    {
                        ctx.v_p[ctx.mapping.key_position_for_message.at(msg_id)] += ctx.v_p.at(ctx.mapping.msg_id_to_msg_position.at(msg_id)) * alpha.at(ctx.mapping.crs_id_for_message.at(msg_id));

                        // std::cout << "v[" << ctx.mapping.key_position_for_message.at(msg_id) << "] += alpha_" << ctx.mapping.crs_id_for_message.at(msg_id) << " * v[" << ctx.mapping.msg_id_to_msg_position.at(msg_id) << "]" << std::endl;

                        msg_id++;
                    }
                }

                ctx.v_p[ctx.crs.n - 1] %= G::order();

                for (const auto& it : ctx.mapping.key_position_for_message)
                {
                    ctx.v_p[it.second] %= G::order();
                }
            }

            for (u32 key_id : ctx.mapping.unique_key_indices)
            {
                // std::cout << "Gamma_" << key_id << ":" << std::endl;

                math::SparseVector<BN> a;
                math::SparseVector<BN> b;
                a.set(0, 1);
                b.set(key_id, -1);
                // std::cout << "- v[" << key_id << "]" << std::endl;
                for (const auto& msg_id : ctx.mapping.messages_for_key.at(key_id))
                {
                    b.set(ctx.mapping.msg_id_to_msg_position.at(msg_id), alpha.at(ctx.mapping.crs_id_for_message.at(msg_id)));

                    // std::cout << "+ alpha_" << ctx.mapping.crs_id_for_message.at(msg_id) << " * v[" << ctx.mapping.msg_id_to_msg_position.at(msg_id) << "]" <<std::endl;
                }
                SparseMatrix mat;
                mat.vectors.emplace_back(a, b);
                ctx.matrices.push_back(mat);
            }

            ctx.state = 2;
            return true;
        }
        else if (ctx.state == 2)
        {
            if (!inner::step_prover(ctx, buffer))
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
        // std::cout << "qesa_copy::step_verifier " << ctx.state << std::endl;
        if (ctx.state == 0)
        {
            Deserializer deserializer(buffer);
            deserializer >> ctx.c_p;
            if (deserializer.available() > 0) throw std::runtime_error("additional unused data found");

            std::vector<BN> alpha;
            for (u32 i = 0; i < ctx.commitments.size(); ++i)
            {
                alpha.push_back(BN::rand());
            }

            Serializer(buffer) << alpha;

            // both sides adjust their states

            for (u32 i = 0; i < ctx.commitments.size(); ++i)
            {
                ctx.c_p += ctx.commitments.at(i) * alpha.at(i);
            }

            for (u32 key_id : ctx.mapping.unique_key_indices)
            {
                math::SparseVector<BN> a;
                math::SparseVector<BN> b;
                a.set(0, 1);
                b.set(key_id, -1);
                for (const auto& msg_id : ctx.mapping.messages_for_key.at(key_id))
                {
                    b.set(ctx.mapping.msg_id_to_msg_position.at(msg_id), alpha.at(ctx.mapping.crs_id_for_message.at(msg_id)));
                }
                SparseMatrix mat;
                mat.vectors.emplace_back(a, b);
                ctx.matrices.push_back(mat);
            }

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
