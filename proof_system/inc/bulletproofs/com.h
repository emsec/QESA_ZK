#pragma once

#include "types.h"
#include "util.h"
#include "math/group.h"

namespace COM
{
    using namespace Group;

    // ##################################
    // types
    // ##################################
    struct CRS
    {
        std::vector<G> H;
    };

    // ##################################
    // methods
    // ##################################

    CRS setup(u32 l);

    G commit(const CRS& crs, const std::vector<BN>& m, const BN& r);

    bool open(const CRS& crs, const std::vector<BN>& m, const BN& r, const G& com);
}
