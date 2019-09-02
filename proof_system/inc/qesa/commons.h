#pragma once

#include "settings.h"

#include "math/sparse_vector.h"
#include "math/group.h"
#include "math/serializer.h"
#include "math/deserializer.h"

#include "util.h"
#include "types.h"

#include <tuple>
#include <exception>

namespace qesa
{
    using namespace Group;

    // ##################################
    // types
    // ##################################

    struct CRS
    {
        math::vector<G> g_p;
        math::vector<G> g_pp;
        G Q;
        u32 n;
    };

    struct SparseMatrix
    {
        // Vector of SparseVector pairs
        // Cf. Section Implementation, paragraph "Representing Gamma" in the paper for more details
        std::vector<  std::pair<  math::SparseVector<BN>, math::SparseVector<BN>  >  > vectors;

        friend std::ostream& operator<<(std::ostream& stream, const SparseMatrix& x);

        friend Serializer& operator<<(Serializer& serializer, const SparseMatrix& x);
        friend Deserializer& operator>>(Deserializer& deserializer, SparseMatrix& x);
    };

    // ##################################
    // methods
    // ##################################

    CRS gen_CRS(u32 witness_size);

}
