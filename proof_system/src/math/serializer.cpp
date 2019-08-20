#include "math/serializer.h"

namespace Group
{
    Serializer::Serializer(std::vector<u8>& buffer, bool append) : m_buffer(buffer)
    {
        if (!append) m_buffer.clear();
    }

    Serializer& Serializer::operator<<(u8 x)
    {
        m_buffer.push_back(x);

        return *this;
    }

    Serializer& Serializer::operator<<(u32 x)
    {
        m_buffer.push_back((x >> 24) & 0xFF);
        m_buffer.push_back((x >> 16) & 0xFF);
        m_buffer.push_back((x >> 8) & 0xFF);
        m_buffer.push_back(x & 0xFF);

        return *this;
    }

    Serializer& Serializer::operator<<(const BN& x)
    {
        u8 tmp[x.size()];
        u32 length = x.serialize(tmp);
        m_buffer.insert(m_buffer.end(), tmp, tmp + length);

        return *this;
    }

    Serializer& Serializer::operator<<(const G& x)
    {
        u8 tmp[x.size()];
        u32 length = x.serialize(tmp);
        m_buffer.insert(m_buffer.end(), tmp, tmp + length);

        return *this;
    }
}
