#pragma once

#include "types.h"
#include "math/vector.h"
#include <tuple>

namespace lmpa
{
    template <typename T>
    class Matrix
    {
    public:
        Matrix()
        {
            m_rows = 0;
            m_cols = 0;
        }

        Matrix(u32 rows, u32 cols)
        {
            m_data = std::vector<T>(rows * cols, 0);
            m_rows = rows;
            m_cols = cols;
        }

        T& operator() (u32 row, u32 col)
        {
            if (row >= rows() || col >= cols())
            {
                throw std::runtime_error("matrix-access out of bounds");
            }
            return m_data.at(to_index(row, col));
        }

        const T& operator() (u32 row, u32 col) const
        {
            if (row >= rows() || col >= cols())
            {
                throw std::runtime_error("matrix-access out of bounds");
            }
            return m_data.at(to_index(row, col));
        }

        u32 rows() const
        {
            return m_rows;
        }

        u32 cols() const
        {
            return m_cols;
        }

        std::tuple<typename std::vector<T>::const_iterator, typename std::vector<T>::const_iterator> get_row_part(u32 row, u32 begin_column, u32 end_column) const
        {
            if (row >= rows() || begin_column >= cols() || end_column > cols())
            {
                throw std::runtime_error("matrix-access out of bounds");
            }
            return {m_data.cbegin() + row * m_cols + begin_column, m_data.cbegin() + row * m_cols + end_column};
        }

        template <typename R>
        math::vector<T> operator* (const math::vector<R>& v) const
        {
            if (v.size() != cols())
            {
                throw std::runtime_error("matrix-vector product size mismatch");
            }

            math::vector<T> res;
            for (u32 row = 0; row < m_rows; ++ row)
            {
                T tmp = 0;
                auto [begin, end] = get_row_part(row, 0, cols());
                auto v_it = v.begin();
                for (auto it = begin; it != end; it++)
                {
                    tmp += (*it) * (*v_it);
                    v_it++;
                }
                res.push_back(tmp);
            }
            return res;
        }

    private:
        // data is inserted row-wise
        inline u32 to_index(u32 row, u32 col)
        {
            return row * m_cols + col;
        }

        std::vector<T> m_data;
        u32 m_rows;
        u32 m_cols;
    };

}
