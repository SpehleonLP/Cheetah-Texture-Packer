#include "imagemanager.h"

bool ImageManager::RemoveImage(Image * it)
{
	std::lock_guard<std::mutex> lock(g_mutex);

	auto itr = loadedImages.find(m_path));

	if(itr != loadedImages.end())
		loadedImages.erase(itr);
}
