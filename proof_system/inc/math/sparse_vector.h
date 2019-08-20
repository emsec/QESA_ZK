#pragma once

#include "math/vector.h"
#include <iostream>
#include <map>
#include <stdexcept>

namespace math
{
    template<class T>
    class SparseVector
    {
    private:

        struct entry
        {
            size_t id;
            T value;
            entry* next;
        };
    public:
        SparseVector()
        {
            m_first = nullptr;
            m_last = nullptr;
            m_item_count = 0;
        }

        SparseVector(const SparseVector<T>& init)
        {
            m_first = nullptr;
            m_last = nullptr;
            m_item_count = 0;
            for (auto it = init.m_first; it != nullptr; it = it->next)
            {
                if (it->value != 0)
                {
                    m_item_count++;
                    auto e = new entry;
                    e->id = it->id;
                    e->value = it->value;
                    e->next = nullptr;
                    if (m_last == nullptr)
                    {
                        m_first = m_last = e;
                    }
                    else
                    {
                        m_last->next = e;
                        m_last = e;
                    }
                }
            }
        }

        SparseVector(const vector<T>& init)
        {
            m_first = nullptr;
            m_last = nullptr;
            m_item_count = 0;
            for (size_t i = 0; i < init.size(); ++i)
            {
                if (init.at(i) != 0)
                {
                    m_item_count++;
                    auto e = new entry;
                    e->id = i;
                    e->value = init.at(i);
                    e->next = nullptr;
                    if (m_last == nullptr)
                    {
                        m_first = m_last = e;
                    }
                    else
                    {
                        m_last->next = e;
                        m_last = e;
                    }
                }
            }
        }

        ~SparseVector()
        {
            clear();
        }

        void clear()
        {
            auto it = m_first;
            while (it != nullptr)
            {
                auto now = it;
                it = it->next;
                delete now;
            }
            m_first = nullptr;
            m_last = nullptr;
            m_item_count = 0;
        }

        const T& operator[](size_t i) const
        {
            auto it = m_first;
            while (it != nullptr && it->id <= i)
            {
                if (it->id == i)
                {
                    return it->value;
                }
                it = it->next;
            }
            return 0;
        }

        void set(size_t i, const T& val)
        {
            if (m_last == nullptr || m_last->id < i)
            {
                m_item_count++;
                auto e = new entry;
                e->id = i;
                e->value = val;
                e->next = nullptr;
                if (m_last == nullptr)
                {
                    m_first = e;
                    m_last = e;
                }
                else
                {
                    m_last->next = e;
                    m_last = e;
                }
            }
            else if (m_first->id > i)
            {
                m_item_count++;
                auto e = new entry;
                e->id = i;
                e->value = val;
                e->next = m_first;
                m_first = e;
            }
            else if (m_first->id == i)
            {
                m_first->value = val;
            }
            else
            {
                auto prev = m_first;
                auto it = m_first->next;
                while (it != nullptr)
                {
                    if (it->id == i)
                    {
                        it->value = val;
                        return;
                    }
                    else if (it->id > i)
                    {
                        m_item_count++;
                        auto e = new entry;
                        e->id = i;
                        e->value = val;
                        e->next = it;
                        prev->next = e;
                        return;
                    }
                    prev = it;
                    it = it->next;
                }
            }
        }

        T operator *(const SparseVector<T>& x) const
        {
            T res = 0;
            auto it_a = m_first;
            auto it_b = x.m_first;

            while (it_a != nullptr && it_b != nullptr)
            {
                if (it_a->id == it_b->id)
                {
                    res += it_a->value * it_b->value;
                    it_a = it_a->next;
                    it_b = it_b->next;
                }
                else if (it_a->id < it_b->id)
                {
                    it_a = it_a->next;
                }
                else
                {
                    it_b = it_b->next;
                }
            }

            return res;
        }

        T operator *(const vector<T>& x) const
        {
            T res = 0;
            for (auto it = m_first; it != nullptr; it = it->next)
            {
                if (it->id >= x.size()) break;
                res += it->value * x.at(it->id);
            }
            return res;
        }

        template<class R>
        R mul_left(const vector<R>& x) const
        {
            R res = 0;
            for (auto it = m_first; it != nullptr; it = it->next)
            {
                if (it->id >= x.size()) break;
                res += x.at(it->id) * it->value;
            }
            return res;
        }

        SparseVector<T> operator *(const T& x) const
        {
            SparseVector<T> res;
            for (auto it = m_first; it != nullptr; it = it->next)
            {
                res.set(it->id, it->value * x);
            }
            return res;
        }

        SparseVector<T> operator %(const T& x) const
        {
            SparseVector<T> res;
            for (auto it = m_first; it != nullptr; it = it->next)
            {
                res.set(it->id, it->value % x);
            }
            return res;
        }

        void operator *=(const T& x)
        {
            for (auto it = m_first; it != nullptr; it = it->next)
            {
                it->value *= x;
            }
        }

        void operator +=(const SparseVector<T>& x)
        {
            T res = 0;
            auto it_a = m_first;
            auto it_b = x.m_first;

            while (it_a != nullptr && it_b != nullptr)
            {
                // std::cout << it_a->id << "  " << it_b->id << ": ";
                if (it_a->id == it_b->id)
                {
                    // std::cout << "mul" << std::endl;
                    it_a->value += it_b->value;
                    it_a = it_a->next;
                    it_b = it_b->next;
                }
                else if (it_a->id < it_b->id)
                {
                    // std::cout << "nop" << std::endl;
                    it_a = it_a->next;
                }
                else
                {
                    this->set(it_b->id, it_b->value);
                    // std::cout << "nop" << std::endl;
                    it_b = it_b->next;
                }
            }
            while (it_b != nullptr)
            {
                this->set(it_b->id, it_b->value);
                // std::cout << "nop" << std::endl;
                it_b = it_b->next;
            }

        }

        void operator %=(const T& x)
        {
            for (auto it = m_first; it != nullptr; it = it->next)
            {
                it->value %= x;
            }
        }

        size_t non_zero_elements() const
        {
            return m_item_count;
        }

        friend std::ostream& operator<<(std::ostream& stream, const SparseVector<T>& x)
        {
            for (auto it = x.m_first; it != nullptr; it = it->next)
            {
                stream << it->value << "* _" << it->id << "_";
                if (it != x.m_last)
                {
                    stream << " + ";
                }
            }
            return stream;
        }

        class const_iterator
        {
            friend class SparseVector;
        public:
            const T& value () { return m_current->value;}
            size_t index () { return m_current->id;}

            const_iterator& operator++ () { m_current = m_current->next; return *this; }
            const_iterator operator++ (int) { m_current = m_current->next; return *this; }

            bool operator== ( const const_iterator& that ) const { return m_current == that.m_current; }
            bool operator!= ( const const_iterator& that ) const { return !(*this == that) ; }

        private:
            const_iterator(entry* item): m_current(item) {};
            entry* m_current;
        };

        const_iterator begin() const { return const_iterator(m_first); }
        const_iterator end() const { return const_iterator(nullptr); }

    private:
        entry* m_first;
        entry* m_last;
        size_t m_item_count;
    };
}
