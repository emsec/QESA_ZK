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

    /*
     * Create a new CRS for committing to a message vector of length l
     */
    CRS setup(u32 l);

    /*
     * Commits to a message vector using randomness r
     */
    G commit(const CRS& crs, const std::vector<BN>& m, const BN& r);

    /*
     * Returns true if the commitment opens to the given message vector
     */
    bool open(const CRS& crs, const std::vector<BN>& m, const BN& r, const G& com);
}
