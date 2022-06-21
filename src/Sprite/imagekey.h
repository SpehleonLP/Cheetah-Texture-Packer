#ifndef IMAGEKEY_H
#define IMAGEKEY_H
#include "Support/counted_string.h"

struct ImageKey
{
	ImageKey() = default;
	ImageKey(std::string const& path, int index = 0) :
		index(index),
		path(counted_string::MakeShared(path))
	{
	}

	int			   index{};
	counted_string path;

	std::string getFilename() const;
	std::string getDirectory() const;
	std::string getMimeType() const;

	bool empty() const { return path.empty(); }

	bool operator<(ImageKey const& key) const
	{
		auto cmp = path.compare(key.path);

		if(cmp == 0) cmp = index - key.index;

		return cmp < 0;
	}
};

#endif // IMAGEKEY_H
