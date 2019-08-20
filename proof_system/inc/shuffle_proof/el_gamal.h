#pragma once

#include "math/group.h"

namespace el_gamal
{
    using namespace Group;

    // ##################################
    // types
    // ##################################

    using PublicKey = G;

    using SecretKey = BN;

    struct KeyPair
    {
        SecretKey sk;
        PublicKey pk;
    };

    struct Ciphertext
    {
        G c_0;
        G c_1;
    };

    // ##################################
    // methods
    // ##################################

    KeyPair keygen();
    Ciphertext encrypt(const PublicKey& pk, const G& m);
    G decrypt(const SecretKey& sk, const Ciphertext& ct);
}
