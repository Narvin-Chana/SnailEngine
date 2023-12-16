#pragma once
#include <array>

#include "Util/SnailException.h"

namespace Snail
{

    struct VectorFull : SnailException {};

    // WARNING! This should only be used in the context of shader constant buffers
    // This structure assures the size in memory is always >= sizeof(T) * N
    template <class T, size_t N>
    class FixedVector : public std::vector<T>
    {
        using std::vector<T>::resize;
        using std::vector<T>::reserve;
    public:
        FixedVector();
        FixedVector(const std::initializer_list<T>& list);
        void push_back(T&&);
        void push_back(const T&);
        void clear();
        typename std::vector<T>::size_type max_size() const noexcept;
    };

    template <class T, size_t N>
    FixedVector <T, N>::FixedVector()
    {
        std::vector<T>::reserve(N);
    }

    template <class T, size_t N>
    FixedVector<T, N>::FixedVector(const std::initializer_list<T>& list)
        : std::vector<T>{list}
    {
        std::vector<T>::reserve(N);
    }

    template <class T, size_t N>
    void FixedVector <T, N>::push_back(T&& val)
    {
        if (std::vector<T>::size() == N)
            throw VectorFull{};

        std::vector<T>::push_back(std::move(val));
    }

    template <class T, size_t N>
    void FixedVector <T, N>::push_back(const T& val)
    {
        if (std::vector<T>::size() == N)
            throw VectorFull{};

        std::vector<T>::push_back(val);
    }

    template <class T, size_t N>
    void FixedVector<T, N>::clear()
    {
        std::vector<T>::clear();
        std::vector<T>::reserve(N);
    }

    template <class T, size_t N>
    typename std::vector<T>::size_type FixedVector <T, N>::max_size() const noexcept
    {
        return N;
    }

}
