#pragma once
#include <type_traits>
#include <vector>
#include <span>

#include "Core/DataStructures/FixedVector.h"

template<class T>
struct InitialBufferSizeTrait
{
	enum { value = sizeof(T) };
};

template<class T>
static constexpr auto MaxBufferSizeTrait_v = InitialBufferSizeTrait<T>::value;

template<class T>
std::enable_if_t<std::is_trivially_copyable_v<T>, const void*> GetBuffer(const T& data)
{
	return &data;
}

template<class T>
std::enable_if_t<std::is_trivially_copyable_v<T>, unsigned int> GetBufferSize(const T& data)
{
	return sizeof(data);
}

template<class T>
const void* GetBuffer(const std::vector<T>& data)
{
	return data.data();
}

template<class T>
unsigned int GetBufferSize(const std::vector<T>& data)
{
	return static_cast<unsigned int>(sizeof(T) * data.size());
}

template<class T>
const void* GetBuffer(const std::span<T>& data)
{
	return data.data();
}

template<class T>
unsigned int GetBufferSize(const std::span<T>& data)
{
	return static_cast<unsigned int>(sizeof(T) * data.size());
}

template<class T, size_t N>
const void* GetBuffer(const Snail::FixedVector<T, N>& data)
{
    return data.data();
}

template<class T, size_t N>
unsigned int GetBufferSize(const Snail::FixedVector<T, N>& data)
{
    return static_cast<unsigned int>(sizeof(T) * data.capacity());
}