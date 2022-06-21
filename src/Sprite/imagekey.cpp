#include "imagekey.h"
#include "Support/imagesupport.h"


std::string ImageKey::getFilename() const
{
	const std::size_t slash_pos = path.find_last_of("/\\");
	const std::size_t period_pos = path.find_last_of(".");

	if(period_pos != std::string::npos
	&& slash_pos != std::string::npos)
		return path.substr(slash_pos+1, period_pos-(slash_pos+1));

	return {};
}

std::string ImageKey::getDirectory() const
{
	const std::size_t pos = path.find_last_of("/\\");
	if (pos != std::string::npos)
	{
		return path.substr(0, pos);
	}

	return {};
}

std::string ImageKey::getMimeType() const
{
	return IO::MimeType(path.c_str());
}
