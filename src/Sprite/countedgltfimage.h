#ifndef COUNTEDGLTFIMAGE_H
#define COUNTEDGLTFIMAGE_H
#include "Support/counted_ptr.hpp"
#include "Support/counted_string.h"
#include "Support/shared_array.hpp"
#include <fx/gltf.h>
#include <atomic>

class GLViewWidget;

struct CountedImage
{
	class ImageManager
	{
	public:
		ImageManager(GLViewWidget *gl) : gl(gl) {}
		~ImageManager() = default;

		GLViewWidget * GetGL() const { return gl; }

	private:
	friend struct Image;
		std::map<counted_string, CountedImage*> loadedImages;
		GLViewWidget                  *  gl;
	};

	static counted_ptr<CountedImage> FromURI(ImageManager * manager, std::string const& path);

	std::string directory() const;
	std::string filename()  const;

	shared_array<uint8_t> imageBuffer;

	counted_string name;
	counted_string uri;
	counted_string mimeType;

	int Id{0};

protected:
friend class counted_ptr<CountedImage>;
	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) { delete this; } }

private:

	ImageManager                  * m_manager{nullptr};
	mutable std::atomic<int> m_refCount{1};
};

struct CountedSampler : public fx::gltf::Sampler
{
	CountedSampler() = default;
	CountedSampler(fx::gltf::Sampler const & it, int id) :
	fx::gltf::Sampler(it), Id(id) {}

	int Id{0};

protected:
friend class counted_ptr<CountedSampler>;
	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) { delete this; } }

private:
	mutable std::atomic<int> m_refCount{1};
};

struct CountedTexture
{
	CountedTexture() = default;

	operator fx::gltf::Texture ()
	{
		fx::gltf::Texture r;

		r.name = name.toStdString();
		r.sampler = sampler != nullptr? sampler->Id : -1;
		r.source  = source != nullptr? source->Id : -1;

		return r;
	}

	counted_string              name;
	counted_ptr<CountedSampler> sampler;
	counted_ptr<CountedImage>   source;

	int Id{0};

protected:
friend class counted_ptr<CountedTexture>;
	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) { delete this; } }

private:
	mutable std::atomic<int> m_refCount{1};
};

#endif // COUNTEDGLTFIMAGE_H
