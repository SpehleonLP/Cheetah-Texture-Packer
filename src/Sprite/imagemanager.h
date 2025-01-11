#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H
#include "imagekey.h"
#include "Support/counted_ptr.hpp"
#include <map>
#include <mutex>
#include <atomic>

class GLViewWidget;
class Image;
struct ImageKey;
class UnpackMemo;
namespace Sprites { struct Document; }

class ImageManager
{
public:
	static counted_ptr<ImageManager> Factory(GLViewWidget *gl) { return UncountedWrap(new ImageManager(gl)); }

	GLViewWidget * GetGL() const { return gl; }

	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) { delete this; } }

	bool RemoveImage(const Image & it);
	counted_ptr<Image> GetImage(ImageKey const& it);
	counted_ptr<Image> GetImage(UnpackMemo & memo, Sprites::Document const& doc, int source, int texCoords);

private:
	ImageManager(GLViewWidget *gl) : gl(gl) {}
	~ImageManager() = default;

	std::mutex					 m_mutex;

	mutable std::atomic<int>	  m_refCount;
	std::map<ImageKey, Image*>    loadedImages;
	GLViewWidget               *  gl;
};


#endif // IMAGEMANAGER_H
