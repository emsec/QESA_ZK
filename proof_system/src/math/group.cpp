#include "math/group.h"

namespace Group
{
 // helper to automatically initialize/free the RELIC core
    namespace
    {
        class RelicHelper
        {
        public:
            RelicHelper()
            {
                if (core_init() != STS_OK)
                {
                    core_clean();
                    return;
                }
                ep_param_set_any();
            };
            ~RelicHelper()
            {
                core_clean();
            };
        };

        static RelicHelper __helper;
    }

#define init_bn(A) {bn_null(A); bn_new(A);}
#define init_G(A) {ep_null(A); ep_new(A);}

// ugly but required hack as RELIC is quite inconsistent with const sometimes
#define UNCONST(type, var) (*(type*)&(var))

// ############################################
//     BN
// ############################################


    void BN::rand(BN& x)
    {
        BN mod = G::order();
        bn_rand_mod(x.element, mod.element);
    }

    void BN::rand(BN& x, int bits, bool positive)
    {
        bn_rand(x.element, positive ? BN_POS : BN_NEG, bits);
    }

    BN BN::rand()
    {
        BN res;
        rand(res);
        return res;
    }

    BN BN::rand(int bits, bool positive)
    {
        BN res;
        rand(res, bits, positive);
        return res;
    }

    void BN::add(BN& d, const BN& x, const BN& y)
    {
        bn_add(d.element, x.element, y.element);
    }

    void BN::sub(BN& d, const BN& x, const BN& y)
    {
        bn_sub(d.element, x.element, y.element);
    }

    void BN::mul(BN& d, const BN& x, const BN& y)
    {
        if (x.is_zero() || y.is_zero())
        {
            d = 0;
            return;
        }
        bn_mul(d.element, x.element, y.element);
    }

    void BN::div(BN& d, const BN& x, const BN& y)
    {
        bn_div(d.element, x.element, y.element);
    }

    void BN::neg(BN& d, const BN& x)
    {
        bn_neg(d.element, x.element);
    }

    void BN::mod(BN& d, const BN& x, const BN& mod)
    {
        // only reduce if necessary
        if ((x >= mod) || (bn_sign(x.element) == BN_NEG))
        {
            bn_mod(d.element, x.element, mod.element);
        }
        else if (&x != &d)
        {
            d = x;
        }
    }

    void BN::shl(BN& d, const BN& x, int bits)
    {
        bn_lsh(d.element, x.element, bits);
    }

    void BN::shr(BN& d, const BN& x, int bits)
    {
        bn_rsh(d.element, x.element, bits);
    }

    void BN::mod_inverse(BN& d, const BN& x, const BN& mod)
    {
        BN tmp;
        BN::mod(tmp, x, mod);
        bn_gcd_ext(tmp.element, d.element, NULL, tmp.element, mod.element);
        if (bn_sign(d.element) == BN_NEG)
        {
            bn_add(d.element, d.element, mod.element);
        }
    }

    BN::BN(BN&& other)
    {
        element[0] = std::move(other.element[0]);
    }
    BN& BN::operator=(BN&& other)
    {
        if (this != &other)
        {
            element[0] = std::move(other.element[0]);
        }
        return *this;
    }
    BN::BN()
    {
        init_bn(element);
    }
    BN::BN(const std::string& hex)
    {
        init_bn(element);
        bn_read_str(element, hex.c_str(), hex.size(), 16);
    }
    BN::BN(const BN& x)
    {
        init_bn(element);
        bn_copy(element, x.element);
    }
    BN::BN(const uint8_t* big_endian_bytes, uint32_t len, bool positive)
    {
        init_bn(element);
        bn_read_bin(element, big_endian_bytes, len);
        if (!positive)
        {
            neg(*this, *this);
        }
    }
    BN::BN(int32_t x)
    {
        init_bn(element);
        *this = x;
    }
    BN::BN(uint32_t x)
    {
        init_bn(element);
        *this = x;
    }
    BN::~BN()
    {
        bn_free(element);
    }

    bool BN::is_zero() const
    {
        return (bn_is_zero(element) == 1);
    }

    void BN::operator =(int32_t x)
    {
        if (x < 0)
        {
            bn_set_dig(element, -x);
            bn_neg(element, element);
        }
        else
        {
            bn_set_dig(element, x);
        }
    }

    void BN::operator =(uint32_t x)
    {
        bn_set_dig(element, x);
    }

    void BN::operator =(const BN& x)
    {
        bn_copy(element, x.element);
    }

    uint32_t BN::bitlength() const
    {
        return bn_bits(element);
    }

    uint32_t BN::bit(uint32_t index) const
    {
        return bn_get_bit(element, index);
    }

    BN BN::operator +(const BN& x) const
    {
        BN res;
        add(res, *this, x);
        return res;
    }

    BN BN::operator -(const BN& x) const
    {
        BN res;
        sub(res, *this, x);
        return res;
    }

    BN BN::operator -() const
    {
        BN res;
        neg(res, *this);
        return res;
    }

    BN BN::operator *(const BN& x) const
    {
        BN res;
        mul(res, *this, x);
        return res;
    }

    G BN::operator *(const G& x) const
    {
        G res;
        G::mul(res, x, *this);
        return res;
    }

    BN BN::operator /(const BN& x) const
    {
        BN res;
        div(res, *this, x);
        return res;
    }

    BN BN::operator %(const BN& x) const
    {
        BN res;
        mod(res, *this, x);
        return res;
    }

    BN BN::operator <<(int bits) const
    {
        BN res;
        shl(res, *this, bits);
        return res;
    }

    BN BN::operator >>(int bits) const
    {
        BN res;
        shr(res, *this, bits);
        return res;
    }


    void BN::operator +=(const BN& x)
    {
        add(*this, *this, x);
    }

    void BN::operator -=(const BN& x)
    {
        sub(*this, *this, x);
    }

    void BN::operator *=(const BN& x)
    {
        mul(*this, *this, x);
    }

    void BN::operator /=(const BN& x)
    {
        div(*this, *this, x);
    }

    void BN::operator %=(const BN& x)
    {
        mod(*this, *this, x);
    }

    void BN::operator <<=(int bits)
    {
        shl(*this, *this, bits);
    }

    void BN::operator >>=(int bits)
    {
        shr(*this, *this, bits);
    }


    BN BN::mod_inverse(const BN& mod) const
    {
        BN res;
        mod_inverse(res, *this, mod);
        return res;
    }

    bool BN::operator >(const BN& x) const
    {
        return bn_cmp(element, x.element) == CMP_GT;
    }

    bool BN::operator <(const BN& x) const
    {
        return bn_cmp(element, x.element) == CMP_LT;
    }

    bool BN::operator ==(const BN& x) const
    {
        return bn_cmp(element, x.element) == CMP_EQ;
    }

    bool BN::operator !=(const BN& x) const
    {
        return !(*this == x);
    }

    bool BN::operator >=(const BN& x) const
    {
        int cmp = bn_cmp(element, x.element);
        return (cmp == CMP_EQ) || (cmp == CMP_GT);
    }

    bool BN::operator <=(const BN& x) const
    {
        int cmp = bn_cmp(element, x.element);
        return (cmp == CMP_EQ) || (cmp == CMP_LT);
    }

    uint16_t BN::size() const
    {
        return bn_size_bin(element) + 3;
    }

    int BN::serialize(uint8_t *buffer) const
    {
        uint16_t len = bn_size_bin(element);
        buffer[0] = (uint8_t)(len >> 8);
        buffer[1] = (uint8_t)(len);
        buffer[2] = (bn_sign(element) == BN_POS) ? 0 : 1;
        bn_write_bin(buffer + 3, len, element);
        return len + 3;
    }

    int BN::deserialize(uint8_t *buffer)
    {
        uint16_t len = (uint16_t)buffer[0] << 8 | (uint16_t)buffer[1];
        bool negative = buffer[2] == 1;
        bn_read_bin(element, buffer + 3, len);
        if (negative)
        {
            neg(*this, *this);
        }
        return len + 3;
    }

    std::ostream& operator<<(std::ostream& stream, const BN& x)
    {
        auto len = bn_size_str(x.element, 16);
        if (len == 0)
        {
            stream << "0";
        }
        else
        {
            char buf[len];
            bn_write_str(buf, len, x.element, 16);
            stream << std::string(buf);
        }
        return stream;
    }


// ############################################
//     G
// ############################################

    G G::infty;
    G G::gen;
    BN G::order_element;
    bool G::initialized_infty = false;
    bool G::initialized_gen = false;
    bool G::initialized_order = false;

    const G& G::get_infty()
    {
        if (!initialized_infty)
        {
            ep_set_infty(infty.element);
            initialized_infty = true;
        }
        return infty;
    }

    const G& G::get_gen()
    {
        if (!initialized_gen)
        {
            ep_curve_get_gen(gen.element);
            initialized_gen = true;
        }
        return gen;
    }

    const BN& G::order()
    {
        if (!initialized_order)
        {
            ep_curve_get_ord(order_element.element);
            initialized_order = true;
        }
        return order_element;
    }

    void G::rand(G& x)
    {
        ep_rand(x.element);
    }

    G G::rand()
    {
        G res;
        rand(res);
        return res;
    }

    void G::mul(G& d, const G& x, const BN& k)
    {
        if (&x == &G::gen)
        {
            ep_mul_gen(d.element, k.element);
        }
        else
        {
            ep_mul(d.element, x.element, k.element);
        }
    }

    void G::add(G& d, const G& x, const G& y)
    {
        ep_add(d.element, x.element, y.element);
    }

    void G::sub(G& d, const G& x, const G& y)
    {
        ep_sub(d.element, x.element, y.element);
    }

    void G::mul_gen(G& x, const BN& k)
    {
        ep_mul_gen(x.element, k.element);
    }

    void G::neg(G& d, const G& x)
    {
        ep_neg(d.element, x.element);
    }

    G::G(G&& other)
    {
        element[0] = std::move(other.element[0]);
    }
    G& G::operator=(G&& other)
    {
        if (this != &other)
        {
            element[0] = std::move(other.element[0]);
        }
        return *this;
    }
    G::G()
    {
        init_G(element);
    }
    G::G(int x)
    {
        init_G(element);
        if (x == 0)
        {
            ep_copy(element, get_infty().element);
        }
        else
        {
            throw std::runtime_error("cannot assign any other value than 0 to a G element.");
        }
    }

    G::G(const G& x)
    {
        init_G(element);
        ep_copy(element, x.element);
    }

    G::~G()
    {
        ep_free(element);
    }

    bool G::is_infty() const
    {
        return ep_is_infty(element) == 1;
    }


    G& G::operator =(const G& x)
    {
        ep_copy(element, x.element);
        return *this;
    }


    G G::operator +(const G& x) const
    {
        G res;
        add(res, *this, x);
        return res;
    }

    G G::operator -(const G& x) const
    {
        G res;
        sub(res, *this, x);
        return res;
    }

    G G::operator -() const
    {
        G res;
        neg(res, *this);
        return res;
    }

    G G::operator *(const BN& k) const
    {
        G res;
        mul(res, *this, k);
        return res;
    }


    void G::operator +=(const G& x)
    {
        add(*this, *this, x);
    }

    void G::operator -=(const G& x)
    {
        sub(*this, *this, x);
    }

    void G::operator *=(const BN& k)
    {
        mul(*this, *this, k);
    }




    bool G::operator ==(const G& x) const
    {
        return ep_cmp(element, x.element) == CMP_EQ;
    }

    bool G::operator !=(const G& x) const
    {
        return !(*this == x);
    }

    uint16_t G::size() const
    {
        if (is_infty())
        {
            return 2;
        }
        return ep_size_bin(element, true) + 2;
    }

    int G::serialize(uint8_t *buffer) const
    {
        uint16_t len = ep_size_bin(element, true);
        if (is_infty())
        {
            buffer[0] = 0;
            buffer[1] = 0;
            return 2;
        }
        buffer[0] = (uint8_t)(len >> 8);
        buffer[1] = (uint8_t)(len);
        ep_write_bin(buffer + 2, len, element, true);
        return len + 2;
    }

    int G::deserialize(uint8_t *buffer)
    {
        uint16_t size = (uint16_t)buffer[0] << 8 | buffer[1];
        if (size == 0)
        {
            ep_set_infty(element);
            return 2;
        }
        ep_read_bin(element, buffer + 2, size);
        return size + 2;
    }

    std::ostream& operator<<(std::ostream& stream, const G& x)
    {
        stream << std::endl;

        G pr = x;
        ep_norm(pr.element, pr.element);
        ep_print(pr.element);

        return stream;
    }
}


