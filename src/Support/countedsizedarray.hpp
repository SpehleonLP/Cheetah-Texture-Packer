#ifndef SIZEDCOUNTEDARRAY_H
#define SIZEDCOUNTEDARRAY_H
#include "counted_ptr.hpp"
#include <cstring>
#include <atomic>

template<typename T>
struct CountedArray
{
	CountedArray(uint32_t size) : m_size(size)
	{
		for(uint32_t i = 1; i < size; ++i)
			new(&m_data[i]) T();
	}
	template<typename S=T>
	CountedArray(uint32_t size, S value) : m_size(size)
	{
		m_data[0] = value;
		for(uint32_t i = 1; i < size; ++i)
		{
			new(&m_data[i]) T();
			m_data[i] = value;
		}
	}
	~CountedArray()
	{
		for(uint32_t i = 1; i < m_size; ++i)
			m_data[i].~T();
	}

	inline void AddRef() { ++m_refCount; }
	inline void Release() { if(--m_refCount == 0) { this->~CountedArray(); std::free(this); } }

	uint32_t         pad00{0};
	uint32_t         m_bytesPerElement{sizeof(T)};
	std::atomic<int> m_refCount{1};
	const uint32_t   m_size{0};
	T                m_data[1];
};

template<typename T>
class ConstSizedArray;

template<typename T>
class CountedSizedArray
{
typedef CountedArray<T> Array;
friend class ConstSizedArray<T>;
public:
friend class asFixedArray;
typedef T value_type;
typedef CountedSizedArray<T> self_type;

	CountedSizedArray() = default;

	CountedSizedArray(counted_ptr<Array> && array) : m_array(std::move(array)) { }

	explicit CountedSizedArray(size_t size)
	{
		if(size)
			m_array = UncountedWrap(new(std::malloc(sizeof(Array) + sizeof(T) * (size-1))) Array(size));
	}

	template<typename S=T>
	CountedSizedArray(size_t size, S value)
	{
		if(size)
			m_array = UncountedWrap(new(std::malloc(sizeof(Array) + sizeof(T) * (size-1))) Array(size, value));
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

	CountedSizedArray clone() const
	{
		if(empty())
			return *this;

		return FromArray(&at(0), size());
	}


	bool operator==(self_type const& it) const { return m_array == it.m_array; }
	bool operator!=(self_type const& it) const { return m_array != it.m_array; }

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

	int GetRefCount() const { return m_array->m_refCount; }

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

	bool merge(self_type const& it)
	{
		if(m_array == it.m_array)
			return true;

		if(m_array->m_size != it.m_array->m_size
		|| memcmp(&m_array->m_data[0], &it.m_array->m_data[0], sizeof(value_type) * m_array->m_size))
			return false;

		m_array = it.m_array;
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

	T       * data()       { return m_array? &m_array->m_data[0] : nullptr; }
	T  const* data() const { return m_array? &m_array->m_data[0] : nullptr; }

private:
	counted_ptr<Array> m_array;
};

template<typename T>
class ConstSizedArray
{
typedef CountedArray<T> Array;
public:
friend class asFixedArray;
typedef T value_type;
typedef ConstSizedArray<T> self_type;
typedef CountedSizedArray<T> non_const_type;

	ConstSizedArray() = default;

	ConstSizedArray(counted_ptr<Array> && array) : m_array(std::move(array)) { }

	template<typename S>
	static self_type FromArray(S * begin, size_t size)
	{
		if(!size) return self_type();

		ConstSizedArray r(size);

		for(size_t i = 0; i < size; ++i)
			r[i] = begin[i];

		return r;
	}

	ConstSizedArray(self_type && it) :
		m_array(std::move(it.m_array)) { }

	ConstSizedArray(self_type const& it) :
		m_array(it.m_array) { }

	ConstSizedArray(non_const_type && it) :
		m_array(std::move(it.m_array)) { }

	ConstSizedArray(non_const_type const& it) :
		m_array(it.m_array) { }

	~ConstSizedArray() = default;

	ConstSizedArray clone() const
	{
		if(empty())
			return *this;

		return FromArray(&at(0), size());
	}

	bool operator==(self_type const& it) const { return m_array == it.m_array; }
	bool operator!=(self_type const& it) const { return m_array != it.m_array; }

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

	self_type const& operator=(non_const_type const& it)
	{
		m_array = it.m_array;
		return *this;
	}

	self_type const& operator=(non_const_type && it)
	{
		m_array = std::move(it.m_array);
		return *this;
	}

	uint32_t size()  const { return m_array? m_array->m_size : 0uL; }
	bool     empty() const { return m_array == nullptr; }
	void     clear() { m_array = counted_ptr<Array>(); }

	int GetRefCount() const { return m_array->m_refCount; }

	const value_type & at(uint32_t i) const { return m_array->m_data[i]; }
	const value_type & operator[](uint32_t i) const { return m_array->m_data[i]; }
	const value_type * begin() const { return &m_array->m_data[0]; }
	const value_type * end() const { return &m_array->m_data[m_array->m_size]; }
	const value_type & front() const { return m_array->m_data[0]; }
	const value_type & back() const { return m_array->m_data[m_array->m_size-1]; }

	void swap(self_type & it)
	{
		m_array.swap(it.m_array);
	}

	bool merge(self_type const& it)
	{
		if(m_array == it.m_array)
			return true;

		if(m_array->m_size != it.m_array->m_size
		|| memcmp(&m_array->m_data[0], &it.m_array->m_data[0], sizeof(value_type) * m_array->m_size))
			return false;

		m_array = it.m_array;
		return true;
	}

	T       * data()       { return m_array? &m_array->m_data[0] : nullptr; }
	T  const* data() const { return m_array? &m_array->m_data[0] : nullptr; }

private:
	counted_ptr<Array> m_array;
};

#endif // SIZEDCOUNTEDARRAY_H
