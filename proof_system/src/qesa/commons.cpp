#include "qesa/commons.h"

namespace qesa
{
    CRS gen_CRS(u32 witness_size)
    {
        CRS crs;
        crs.n = round_to_next_power_of_two(witness_size + 2);
        for (u32 i = 0; i < crs.n; ++i)
        {
            crs.g_p.push_back(G::rand());
            crs.g_pp.push_back(G::rand());
        }
        crs.Q = G::rand();
        return crs;
    }

    std::ostream& operator<<(std::ostream& stream, const SparseMatrix& x)
    {
        for (u32 i = 0; i < x.vectors.size(); ++i)
        {
            stream << "(" << std::get<0>(x.vectors.at(i)) << ") * (" << std::get<1>(x.vectors.at(i)) << ")";
            if (i == x.vectors.size() - 1)
            {
                stream << " = 0";
            }
            else
            {
                stream << " + ";
            }
        }
        return stream;
    }

    Serializer& operator<<(Serializer& serializer, const SparseMatrix& x)
    {
        serializer << (u32) x.vectors.size();
        for (const auto& [a, b] : x.vectors)
        {
            serializer << (u32) a.non_zero_elements();
            for (auto it = a.begin(); it != a.end(); ++it)
            {
                serializer << (u32) it.index() << it.value();
            }

            serializer << (u32) b.non_zero_elements();
            for (auto it = b.begin(); it != b.end(); ++it)
            {
                serializer << (u32) it.index() << it.value();
            }
        }
        return serializer;
    }

    Deserializer& operator>>(Deserializer& deserializer, SparseMatrix& x)
    {
        u32 n;
        deserializer >> n;
        for (u32 i = 0; i < n; ++i)
        {
            u32 index;
            BN value;
            math::SparseVector<BN> a;
            math::SparseVector<BN> b;
            u32 k;

            deserializer >> k;
            for (u32 j = 0; j < k; ++j)
            {
                deserializer >> index >> value;
                a.set(index, value);
            }

            deserializer >> k;
            for (u32 j = 0; j < k; ++j)
            {
                deserializer >> index >> value;
                b.set(index, value);
            }

            x.vectors.emplace_back(a, b);
        }
        return deserializer;
    }
}
