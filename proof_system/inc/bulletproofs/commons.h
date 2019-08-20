#pragma once

#include "settings.h"

#include "math/group.h"
#include "math/vector.h"
#include "math/serializer.h"
#include "math/deserializer.h"

#include "bulletproofs/com.h"

#include "util.h"
#include "types.h"

namespace bulletproofs
{
    using namespace Group;

    // ##################################
    // types
    // ##################################

    struct CRS
    {
        COM::CRS crs_com;

        u32 n;
        u32 m;
        math::vector<G> g;
        math::vector<G> h;
        #if USE_BULLETPROOFS_IPA || USE_QESA_IPA
        G u;
        #endif

    };

    // ##################################
    // methods
    // ##################################

    CRS setup(u32 range, u32 num_commitments, const COM::CRS& crs_com);
}
