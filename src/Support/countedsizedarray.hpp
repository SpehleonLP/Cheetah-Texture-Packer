#ifndef SIZEDCOUNTEDARRAY_H
#define SIZEDCOUNTEDARRAY_H
#include "counted_ptr.hpp"
#include <cstring>
#include <atomic>

template<typename T>
class CountedSizedArray
{
public:
typedef T value_type;
typedef CountedSizedArray<T> self_type;
	CountedSizedArray() = default;

	CountedSizedArray(size_t size)
	{
		if(size)
			m_array = UncountedWrap(new(malloc(sizeof(Array) + sizeof(T) * size)) Array(size));
	}

	CountedSizedArray(size_t size, T value)
	{
		if(size)
			m_array = UncountedWrap(new(malloc(sizeof(Array) + sizeof(T) * size)) Array(size, value));
	}

	template<typename S>
	static self_type FromArray(S * begin, size_t size)
	{
		if(!size) return self_type();

		CountedSizedArray r(size);

		for(size_t i = 0; i < size; ++i)
			r[i] = begin[i];

		return r;
	}

	CountedSizedArray(self_type && it) :
		m_array(std::move(it.m_array)) { }

	CountedSizedArray(self_type const& it) :
		m_array(it.m_array) { }

	~CountedSizedArray() = default;

	bool operator==(self_type const& it) { return m_array == it.m_array; }
	bool operator!=(self_type const& it) { return m_array != it.m_array; }

	self_type const& operator=(self_type const& it)
	{
		m_array = it.m_array;
		return *this;
	}

	self_type const& operator=(self_type && it)
	{
		m_array = std::move(it.m_array);
		return *this;
	}

	uint32_t size()  const { return m_array? m_array->m_size : 0uL; }
	bool     empty() const { return m_array == nullptr; }
	void     clear() { m_array = counted_ptr<Array>(); }

	      value_type & at(uint32_t i) { return m_array->m_data[i]; }
	const value_type & at(uint32_t i) const { return m_array->m_data[i]; }

	      value_type & operator[](uint32_t i) { return m_array->m_data[i]; }
	const value_type & operator[](uint32_t i) const { return m_array->m_data[i]; }

		  value_type * begin()       { return &m_array->m_data[0]; }
	const value_type * begin() const { return &m_array->m_data[0]; }

		  value_type * end()       { return &m_array->m_data[m_array->m_size]; }
	const value_type * end() const { return &m_array->m_data[m_array->m_size]; }

	      value_type & front()       { return m_array->m_data[0]; }
	const value_type & front() const { return m_array->m_data[0]; }

		  value_type & back()       { return m_array->m_data[m_array->m_size-1]; }
	const value_type & back() const { return m_array->m_data[m_array->m_size-1]; }

	void swap(self_type & it)
	{
		m_array.swap(it.m_array);
	}

	bool merge(self_type & it)
	{
		if(m_array == it.m_array)
			return true;

		if(m_array->m_size != it.m_array->m_size
		|| memcmp(&m_array->m_data[0], &it.m_array->m_data[0], sizeof(value_type) * m_array->m_size))
			return false;

		if(m_array->m_refCount <= it.m_array->m_refCount)
		{
			m_array = it.m_array;
		}
		else
		{
			it.m_array = m_array;
		}

		return true;
	}

	bool CanMerge(self_type const& it)
	{
		if(m_array == it.m_array)
			return true;

		if(m_array->m_size != it.m_array->m_size
		|| memcmp(&m_array->m_data[0], &it.m_array->m_data[0], sizeof(value_type) * m_array->m_size))
			return false;

		return true;
	}

	void const* data() const { return m_array == nullptr? nullptr : &m_array->m_data[0]; }

private:
	struct Array
	{
		Array(uint32_t size) : m_size(size)
		{
			for(uint32_t i = 1; i < size; ++i)
				new(&m_data[i]) T();
		}
		Array(uint32_t size, T value) : m_size(size)
		{
			m_data[0] = value;
			for(uint32_t i = 1; i < size; ++i)
			{
				new(&m_data[i]) T();
				m_data[i] = value;
			}
		}
		~Array()
		{
			for(uint32_t i = 1; i < m_size; ++i)
				m_data[i].~T();
		}

		void AddRef() { ++m_refCount; }
		void Release() { if(--m_refCount == 0) { this->~Array(); free(this); } }

		std::atomic<int> m_refCount{1};
		const uint32_t   m_size{0};
		T                m_data[1];
	};

	counted_ptr<Array> m_array;
};

#endif // SIZEDCOUNTEDARRAY_H
