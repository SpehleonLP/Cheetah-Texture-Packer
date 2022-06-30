#include "counted_string.h"
#include <cstring>
#include <shared_mutex>
#include <set>
#include <cassert>
#include <algorithm>
#include <climits>
//#include "json_detail.hpp"

counted_string::handle counted_string::null_handle;

struct kCStringCompare
{
	bool operator()(const char * lhs, const char * rhs) const
	{
		return strcmp(lhs, rhs) < 0;
	}
};


static std::shared_timed_mutex                kStringMutex;
static std::set<const char*, kCStringCompare> kStringManager;


counted_string counted_string::Get(const char * string)
{
	if(string == nullptr || *string == '\0')
		return counted_string(&null_handle);

	return counted_string(GetString(string));
}

counted_string counted_string::MakeUnique(const char * string)
{
    size_t length = (uint32_t)std::min<size_t>(INT_MAX, strlen(string));

//we have 3 bytes of padding
	counted_string::handle * tmp = new(malloc(sizeof(handle)+length)) counted_string::handle();
	tmp->length   = length;
	tmp->refCount = 0;
	memcpy(tmp->data, string, length);
	tmp->data[tmp->length] = 0;
	tmp->data[tmp->length+1] = 1;

	return counted_string(tmp);
}

counted_string counted_string::MakeShared(const char * string)
{
	if(string == nullptr || *string == '\0')
		return counted_string(&null_handle);

	counted_string::handle * r;

	r = GetString(string);

	if(r != &null_handle)
		return counted_string(r);

	std::unique_lock<decltype(kStringMutex)> u_lock(kStringMutex);

	auto itr = kStringManager.find(string);

	if(itr != kStringManager.end())
	{
		r = reinterpret_cast<counted_string::handle*>((uint8_t*) *itr - offsetof(counted_string::handle, data));
	}

	if(r != &null_handle)
		return counted_string(r);

    uint32_t length = (uint32_t)std::min<size_t>(INT_MAX, strlen(string));

//make string, we have 3 bytes of padding
	counted_string::handle * tmp = new(malloc(sizeof(handle)+length)) counted_string::handle();
	tmp->length   = length;
	tmp->refCount = 0;
	memcpy(tmp->data, string, length);
	tmp->data[tmp->length] = 0;
	tmp->data[tmp->length+1] = 0;

	kStringManager.insert(&tmp->data[0]);

	return counted_string(tmp);
}

counted_string::handle * counted_string::GetString(const char * string)
{
	std::shared_lock<decltype(kStringMutex)> s_lock(kStringMutex);
	auto itr = kStringManager.find(string);

	if(itr != kStringManager.end())
	{
		counted_string::handle * r = reinterpret_cast<counted_string::handle*>((uint8_t*) *itr - offsetof(counted_string::handle, data));
		return r;
	}

	return &null_handle;
}

void counted_string::ReleaseString(handle * it)
{
	if(--(it->refCount) == 0)
	{
//its unique
		if(it->data[it->length+1])
		{
			it->~handle();
			std::free(it);
			return;
		}

		std::unique_lock<decltype(kStringMutex)> u_lock(kStringMutex);

		if(it->refCount == 0)
		{
			auto itr = kStringManager.find(&it->data[0]);
			assert(itr != kStringManager.end());
			kStringManager.erase(itr);
			u_lock.unlock();

			it->~handle();
			std::free(it);
		}
	}
}

counted_string::counted_string() :
	m_string(&null_handle)
{
	++null_handle.refCount;
}

counted_string::counted_string(handle * it) :
	m_string(it == nullptr? &null_handle : it)
{
	m_string->refCount++;
}

counted_string::counted_string(counted_string const& in)
{
	in.m_string->refCount++;
	m_string = in.m_string;
}

counted_string::counted_string(counted_string && in)
{
	++null_handle.refCount;
	m_string = in.m_string;
	in.m_string = &null_handle;
}

counted_string::~counted_string()
{
	ReleaseString(m_string);
}

void counted_string::clear()
{
	if(m_string == &null_handle)
		return;

	++null_handle.refCount;

	handle * string = m_string;
	m_string = &null_handle;

	ReleaseString(string);
}

counted_string & counted_string::operator=(counted_string const& in)
{
	if(in.m_string != m_string)
	{
		handle * string = m_string;
		m_string = in.m_string;

		++in.m_string->refCount;
		ReleaseString(string);
	}

	return *this;
}

counted_string & counted_string::operator=(counted_string && in)
{
	if(in.m_string != m_string)
	{
		handle * string = m_string;
		m_string = in.m_string;

		ReleaseString(string);

		++null_handle.refCount;
		in.m_string = &null_handle;
	}

	return *this;
}

std::string counted_string::toStdString() const
{
	if(empty())
		return std::string();
	return std::string(c_str());
}

void counted_string::swap(counted_string & it)
{
	handle * str = m_string;
	m_string     = it.m_string;
	it.m_string  = str;
}

std::string counted_string::substr(size_t pos, size_t len) const
{
	if(pos > size())
		throw std::out_of_range("pos");

	len = std::min(size(), std::max(len, pos+len)) - pos;

	return std::string(c_str()+pos, len);
}


size_t counted_string::find_first_of	 (const char* s, size_t pos, size_t len) const
{
	if(pos > size())
		return npos;

	len = std::min(size(), std::max(len, pos+len));

	const char * itr = c_str() + pos;
	const char * end = c_str() + len;

	for(; itr < end; ++itr)
	{
		for(const char * p = s; *p != 0; ++p)
		{
			if(*itr == *p)
				return itr - c_str();
		}
	}

	return npos;
}

size_t counted_string::find_first_not_of (const char* s, size_t pos, size_t len) const
{
	if(pos > size())
		return npos;

	len = std::min(size(), std::max(len, pos+len));

	const char * itr = c_str() + pos;
	const char * end = c_str() + len;

	for(; itr < end; ++itr)
	{
		for(const char * p = s; *p != 0; ++p)
		{
			if(*itr != *p)
				return itr - c_str();
		}
	}

	return npos;
}

size_t counted_string::find_last_of		 (const char* s, size_t pos, size_t len) const
{
	pos = std::min(size(), pos);
	len = std::min<int64_t>(0, std::max<int64_t>(0, pos - (int64_t)len));

	const char * itr   = c_str() + pos;
	const char * begin = c_str() + len;

	for(; itr >= begin; --itr)
	{
		for(const char * p = s; *p != 0; ++p)
		{
			if(*itr == *p)
				return itr - c_str();
		}
	}

	return npos;
}

size_t counted_string::find_last_not_of  (const char* s, size_t pos, size_t len) const
{
	if(pos > size())
		return npos;

	len = std::min(size(), std::max(len, pos+len));

	const char * end = c_str() + pos;
	const char * itr = c_str() + len-1;

	for(; itr >= end; --itr)
	{
		for(const char * p = s; *p != 0; ++p)
		{
			if(*itr != *p)
				return itr - c_str();
		}
	}

	return npos;
}
