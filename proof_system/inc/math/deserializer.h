#pragma once

#include "types.h"
#include "util.h"
#include "math/group.h"

namespace Group
{
    class Deserializer
    {
    public:
        Deserializer(const std::vector<u8>& buffer);
        ~Deserializer() = default;

        // reads data from byte position pos and advances pos to the next data blob
        Deserializer& operator>>(u8& x);
        Deserializer& operator>>(u32& x);
        Deserializer& operator>>(BN& x);
        Deserializer& operator>>(G& x);

        template <class T>
        Deserializer& operator>>(std::vector<T>& vec)
        {
            check();

            u32 num_elements;
            (*this) >> num_elements;
            vec.clear();
            vec.reserve(num_elements);
            for (u32 i = 0; i < num_elements; ++i)
            {
                T x;
                (*this) >> x;
                vec.push_back(x);
            }
            return *this;
        }

        void set_position(u32 pos);

        u32 get_position();

        u32 available();
    private:
        void check();
        const std::vector<u8>& m_buffer;
        u32 m_pos;
    };
}
