#include "bulletproofs/commons.h"

namespace bulletproofs
{
    CRS setup(u32 range, u32 num_commitments, const COM::CRS& crs_com)
    {
        CRS crs;

        crs.crs_com = crs_com;

        crs.n = range;
        crs.m = num_commitments;

        for (u32 i = 0; i < crs.n * crs.m; ++i)
        {
            crs.g.push_back(G::rand());
            crs.h.push_back(G::rand());
        }

        #if USE_BULLETPROOFS_IPA || USE_QESA_IPA
        crs.u = G::rand();
        #endif

        return crs;
    }
}
