#pragma once

#include "types.h"
#include "util.h"
#include "math/group.h"

namespace Group
{
    class Serializer
    {
    public:
        Serializer(std::vector<u8>& buffer, bool append = false);
        ~Serializer() = default;

        // appends data to the end of the buffer
        Serializer& operator<<(u8 x);
        Serializer& operator<<(u32 x);
        Serializer& operator<<(const BN& x);
        Serializer& operator<<(const G& x);

        template <class T>
        Serializer& operator<<(const std::vector<T>& vec)
        {
            (*this) << (u32)vec.size();
            for (const auto& x : vec) (*this) << x;
            return *this;
        }

    private:
        std::vector<u8>& m_buffer;
    };
}
