#pragma once

#include <vector>
#include <iostream>

namespace math
{
    template<class T>
    class vector : public std::vector<T>
    {
    public:
        using std::vector<T>::vector;

        // inner product
        template<class R>
        T operator *(const vector<R>& x) const
        {
            if (x.size() != this->size())
            {
                throw std::runtime_error("vectors have different size.");
            }

            T res = 0;
            for (size_t i = 0; i < this->size(); ++i)
            {
                res += this->at(i) * x[i];
            }
            return res;
        }


        template<class R>
        vector<T> operator *(const R& x) const
        {
            vector<T> res;
            for (const auto& v : *this)
            {
                res.push_back(v * x);
            }
            return res;
        }

        vector<T> operator %(const T& x) const
        {
            vector<T> res;
            for (const auto& v : *this)
            {
                res.push_back(v % x);
            }
            return res;
        }

        vector<T> operator +(const vector<T>& x) const
        {
            if (x.size() != this->size())
            {
                throw std::runtime_error("vectors have different size.");
            }

            vector<T> res;
            for (size_t i = 0; i < this->size(); ++i)
            {
                res.push_back(this->at(i) + x.at(i));
            }
            return res;
        }

        vector<T> operator -(const vector<T>& x) const
        {
            if (x.size() != this->size())
            {
                throw std::runtime_error("vectors have different size.");
            }

            vector<T> res;
            for (size_t i = 0; i < this->size(); ++i)
            {
                res.push_back(this->at(i) - x.at(i));
            }
            return res;
        }

        template<class R>
        void operator *=(const R& x)
        {
            for (size_t i = 0; i < this->size(); ++i)
            {
                (*this)[i] *= x;
            }
        }

        void operator %=(const T& x)
        {
            for (size_t i = 0; i < this->size(); ++i)
            {
                (*this)[i] %= x;
            }
        }

        void operator +=(const vector<T>& x)
        {
            if (x.size() != this->size())
            {
                throw std::runtime_error("vectors have different size.");
            }

            for (size_t i = 0; i < this->size(); ++i)
            {
                (*this)[i] += x.at(i);
            }
        }

        void operator -=(const vector<T>& x)
        {
            if (x.size() != this->size())
            {
                throw std::runtime_error("vectors have different size.");
            }

            for (size_t i = 0; i < this->size(); ++i)
            {
                (*this)[i] -= x.at(i);
            }
        }

        friend std::ostream& operator<<(std::ostream& stream, const math::vector<T>& x)
        {
            stream << "[";
            for (size_t i = 0; i < x.size(); ++i)
            {
                stream << x.at(i);
                if (i != x.size() - 1)
                {
                    stream << ", ";
                }
            }
            return stream << "]";
        }

    };
}
