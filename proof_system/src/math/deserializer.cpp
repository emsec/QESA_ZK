#include "math/deserializer.h"

namespace Group
{
    Deserializer::Deserializer(const std::vector<u8>& buffer) : m_buffer(buffer)
    {
        m_pos = 0;
    }

    Deserializer& Deserializer::operator>>(u8& x)
    {
        check();
        x = m_buffer.at(m_pos++);
        return *this;
    }

    Deserializer& Deserializer::operator>>(u32& x)
    {
        check();
        x  = m_buffer.at(m_pos++) << 24;
        x |= m_buffer.at(m_pos++) << 16;
        x |= m_buffer.at(m_pos++) << 8;
        x |= m_buffer.at(m_pos++);

        return *this;
    }

    Deserializer& Deserializer::operator>>(BN& x)
    {
        check();
        m_pos += x.deserialize((u8*)(&m_buffer[m_pos]));
        return *this;
    }

    Deserializer& Deserializer::operator>>(G& x)
    {
        check();
        m_pos += x.deserialize((u8*)(&m_buffer[m_pos]));
        return *this;
    }

    void Deserializer::set_position(u32 pos)
    {
        m_pos = pos;
    }

    u32 Deserializer::get_position()
    {
        return m_pos;
    }

    u32 Deserializer::available()
    {
        return m_buffer.size() - m_pos;
    }

    void Deserializer::check()
    {
        if (available() == 0)
        {
            throw std::runtime_error("deserializer is out of data");
        }
    }
}
