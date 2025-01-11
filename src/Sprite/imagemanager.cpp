#include "imagemanager.h"
#include "image.h"
#include "imagetexturecoordinates.h"
#include "Support/unpackmemo.h"
#include "Support/imagesupport.h"


counted_ptr<Image> ImageManager::GetImage(ImageKey const& key)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	auto itr = loadedImages.find(key);

	if(itr != loadedImages.end())
		return CountedWrap(itr->second);

	auto r = UncountedWrap(new Image(CountedWrap(this), key));
	loadedImages[key] = r.get();
	return r;
}

bool ImageManager::RemoveImage(Image const& it)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if(it.GetRefCount() != 0)
		return false;

	auto itr = loadedImages.find(it.key());

	if(itr != loadedImages.end())
	{
		loadedImages.erase(itr);
		return true;
	}

	return true;
}

counted_ptr<Image> ImageManager::GetImage(UnpackMemo & memo, Sprites::Document const& doc, int source, int texCoords)
{
	auto key = ImageKey(memo.documentFilePath, source);

	std::lock_guard<std::mutex> lock(m_mutex);

	auto itr = loadedImages.find(key);

	if(itr != loadedImages.end())
		return CountedWrap(itr->second);

	auto r = UncountedWrap(new Image(CountedWrap(this), key,
		IO::LoadImage(doc, source),
		ImageTextureCoordinates::Factory(doc, texCoords, memo)));

	loadedImages[key] = r.get();
	return r;

}
