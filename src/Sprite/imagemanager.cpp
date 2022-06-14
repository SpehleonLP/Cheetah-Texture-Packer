#include "imagemanager.h"

std::mutex     ImageManager::g_mutex;
ImageManager * ImageManager::g_singleton;

counted_ptr<ImageManager> ImageManager::Factory(GLViewWidget *gl)
{
	std::lock_guard<std::mutex> lock(g_mutex);



}
