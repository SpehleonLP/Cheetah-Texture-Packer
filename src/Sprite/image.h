#ifndef IMAGE_H
#define IMAGE_H
#include "imagekey.h"
#include "Support/counted_string.h"
#include "Support/counted_ptr.hpp"
#include "Support/shared_array.hpp"
#include <glm/gtc/type_precision.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <mutex>
#include <map>
#include <memory>

class GLViewWidget;
#define HAVE_CHROMA_KEY 0

class ImageManager;
class ImageTextureCoordinates;
struct ImageKey;

namespace IO { struct Image; }

class Image
{
public:
	static counted_ptr<Image> Factory(counted_ptr<ImageManager>, std::string const& path);

	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) { Destroy(); } }
	int GetRefCount() const { return m_refCount; }

	bool LoadFromFile();
	void Clear();

	std::unique_ptr<uint8_t[]> LoadFileAsArray(uint32_t & size) const;
	counted_ptr<ImageTextureCoordinates> m_texCoords;

	__always_inline bool isLoaded() const { return m_isLoaded; }
	__always_inline bool hasAlpha() const { return m_hasAlpha; }

	__always_inline uint32_t GetTexture() const { return m_texture; };
	__always_inline glm::u16vec2 GetSize() const { return m_size; }

	__always_inline auto getFilename()	const { return m_key.getFilename(); }
	__always_inline auto getDirectory()	const { return m_key.getDirectory(); }
	__always_inline auto getMimeType()	const { return m_key.getMimeType(); }

	ImageKey const& key() const { return m_key; }

private:
friend class ImageManager;
	Image(counted_ptr<ImageManager>, const ImageKey & key);
	Image(counted_ptr<ImageManager>, const ImageKey & key, IO::Image && image, counted_ptr<ImageTextureCoordinates> &&);
	~Image();

	void LoadImage(IO::Image & image);
	bool Destroy();

	counted_ptr<ImageManager>       m_manager;
	ImageKey						m_key;

	glm::i16vec2                    m_size{0, 0};
	uint8_t                         m_channels{0};

	bool                            m_hasAlpha{0};
	bool                            m_isLoaded{0};
	bool                            m_isLoading{0};
	bool                            m_ownsTexture{0};

	uint32_t                        m_texture{0};
	mutable std::atomic<int>        m_refCount{1};
};

#endif // IMAGE_H
