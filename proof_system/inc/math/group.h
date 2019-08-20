#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "fifo_map.h"

using namespace nlohmann;

extern "C" {
#include "math/relic/relic.h"
}

namespace Group
{
    class G;

    class BN
    {
        friend class G;
    public:
        BN();
        BN(const std::string& hex);
        BN(const uint8_t* big_endian_bytes, uint32_t len, bool positive);
        BN(const BN& x);
        BN(int32_t x);
        BN(uint32_t x);
        ~BN();

        BN(BN&& other);
        BN& operator=(BN&& other);

        bool is_zero() const;

        static void rand(BN& x);
        static BN rand();
        static void rand(BN& x, int bits, bool positive);
        static BN rand(int bits, bool positive);

        static void add(BN& d, const BN& x, const BN& y);
        static void sub(BN& d, const BN& x, const BN& y);
        static void mul(BN& d, const BN& x, const BN& y);
        static void div(BN& d, const BN& x, const BN& y);
        static void neg(BN& d, const BN& x);
        static void mod(BN& d, const BN& x, const BN& mod);
        static void shl(BN& d, const BN& x, int bits);
        static void shr(BN& d, const BN& x, int bits);
        static void mod_inverse(BN& d, const BN& x, const BN& mod);

        void operator =(int32_t x);
        void operator =(uint32_t x);
        void operator =(const BN& x);

        uint32_t bitlength() const;
        uint32_t bit(uint32_t index) const;

        BN operator +(const BN& x) const;
        BN operator -(const BN& x) const;
        BN operator -() const;
        BN operator *(const BN& x) const;
        G operator *(const G& x) const;
        BN operator /(const BN& x) const;
        BN operator %(const BN& x) const;
        BN operator <<(int bits) const;
        BN operator >>(int bits) const;
        void operator +=(const BN& x);
        void operator -=(const BN& x);
        void operator *=(const BN& x);
        void operator /=(const BN& x);
        void operator %=(const BN& x);
        void operator <<=(int bits);
        void operator >>=(int bits);

        BN mod_inverse(const BN& mod) const;

        bool operator >(const BN& x) const;
        bool operator <(const BN& x) const;
        bool operator ==(const BN& x) const;
        bool operator !=(const BN& x) const;
        bool operator >=(const BN& x) const;
        bool operator <=(const BN& x) const;

        uint16_t size() const;
        int serialize(uint8_t *buffer) const;
        int deserialize(uint8_t *buffer);

        friend std::ostream& operator<< (std::ostream& stream, const BN& x);

    private:
        bn_t element;
    };

    class G
    {
    public:
        static const G& get_infty();
        static const G& get_gen();
        static const BN& order();

        static void rand(G& x);
        static G rand();

        static void mul_gen(G& x, const BN& k);
        static void mul(G& d, const G& x, const BN& k);
        static void add(G& d, const G& x, const G& y);
        static void sub(G& d, const G& x, const G& y);
        static void neg(G& d, const G& x);

        G();
        G(int x);
        G(const G& x);
        ~G();
        G(G&& other);
        G& operator=(G&& other);

        bool is_infty() const;

        G& operator =(const G& x);

        G operator +(const G& x) const;
        G operator -(const G& x) const;
        G operator -() const;
        G operator *(const BN& x) const;

        void operator +=(const G& x);
        void operator -=(const G& x);
        void operator *=(const BN& k);

        bool operator ==(const G& x) const;
        bool operator !=(const G& x) const;

        uint16_t size() const;
        int serialize(uint8_t *buffer) const;
        int deserialize(uint8_t *buffer);

        friend std::ostream& operator<< (std::ostream& stream, const G& x);

    private:
        ep_t element;

        static G gen;
        static G infty;
        static BN order_element;
        static bool initialized_infty;
        static bool initialized_order;
        static bool initialized_gen;
    };
}
