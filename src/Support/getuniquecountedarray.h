#ifndef GETUNIQUECOUNTEDARRAY_H
#define GETUNIQUECOUNTEDARRAY_H
#include "countedsizedarray.hpp"
#include <cstring>
#include <mutex>

struct CountedDBBase
{
	static std::mutex g_mutex;
};

template<typename T>
inline ConstSizedArray<T> MakeUnique(ConstSizedArray<T> const& array)
{
	static std::vector<ConstSizedArray<T>> g_database;

	if(array.empty())
		return array;

	std::lock_guard<std::mutex> lock(CountedDBBase::g_mutex);

	for(auto i = 0u; i < g_database.size(); ++i)
	{
		if(g_database[i] == array)
			return g_database[i];

		if(g_database[i].size() == array.size()
		&& 0 == memcmp(g_database[i].data(), array.data(), sizeof(T) * array.size()))
		{
			return g_database[i];
		}
	}

	g_database.push_back(array);
	return array;
}



#endif // GETUNIQUECOUNTEDARRAY_H
