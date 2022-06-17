#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H
#include "Support/counted_ptr.hpp"
#include <map>
#include <mutex>
#include <atomic>

class GLViewWidget;
class Image;
struct ImageKey;

class ImageManager
{
public:
	static counted_ptr<ImageManager> Factory(GLViewWidget *gl) { return UncountedWrap(new ImageManager(gl)); }

	GLViewWidget * GetGL() const { return gl; }

	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) { delete this; } }

	bool RemoveImage(Image * it);
	counted_ptr<Image> GetImage(Image * it);

private:
	ImageManager(GLViewWidget *gl) : gl(gl) {}
	~ImageManager() = default;

	std::mutex					 m_mutex;

	struct Key
	{

	};

	mutable std::atomic<int>	  m_refCount;
	std::map<std::string, Image*> loadedImages;
	GLViewWidget               *  gl;
};


#endif // IMAGEMANAGER_H
