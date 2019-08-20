#include "shuffle_proof/el_gamal.h"

namespace el_gamal
{
    KeyPair keygen()
    {
        KeyPair keys;
        keys.sk = BN::rand();
        keys.pk = G::get_gen() * keys.sk;
        return keys;
    }

    Ciphertext encrypt(const PublicKey& pk, const G& m)
    {
        Ciphertext ct;
        BN r = BN::rand();
        ct.c_0 = G::get_gen() * r;
        ct.c_1 = pk * r + m;
        return ct;
    }

    G decrypt(const SecretKey& sk, const Ciphertext& ct)
    {
        return ct.c_0 * (-sk) + ct.c_1;
    }
}
