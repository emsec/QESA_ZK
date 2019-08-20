#include "bulletproofs/com.h"

namespace COM
{
    CRS setup(u32 l)
    {
        CRS crs;
        for (u32 i = 0; i < l; ++i)
        {
            crs.H.push_back(G::rand());
        }
        return crs;
    }

    G commit(const CRS& crs, const std::vector<BN>& m, const BN& r)
    {
        if ( crs.H.size() < m.size() )
        {
            throw std::runtime_error("COM::commit: wrong message length");
        }

        G com = G::get_gen() * r;

        for (u32 i = 0; i < m.size(); ++i)
        {
            if (!m[i].is_zero())
            {
                com += crs.H[i] * m[i];
            }
        }
        return com;
    }

    bool open(const CRS& crs, const std::vector<BN>& m, const BN& r, const G& com)
    {
        if ( crs.H.size() < m.size() )
        {
            throw std::runtime_error("COM::open: wrong message length");
        }

        auto tmp_com = commit(crs, m, r);

        return (tmp_com == com);
    }

}
