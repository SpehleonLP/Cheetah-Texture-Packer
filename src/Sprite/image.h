#ifndef IMAGE_H
#define IMAGE_H
#include "Support/counted_string.h"
#include "Support/countedsizedarray.hpp"
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
	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) { Clear(); delete this; } }

	void LoadFromFile();
	void Clear();

	std::unique_ptr<uint8_t[]> LoadFileAsArray(uint32_t & size) const;
	counted_ptr<ImageTextureCoordinates> m_texCoords;

	bool isLoaded() const { return m_isLoaded; }
	bool hasAlpha() const { return m_hasAlpha; }

	uint32_t GetTexture() const { return m_texture; };
	glm::u16vec2 GetSize() const { return m_size; }

	std::string getFilename() const;
	std::string getDirectory() const;
	std::string getMimeType() const;

private:
	Image(counted_ptr<ImageManager>, std::string const& path, IO::Image & image);
	~Image();

	static std::mutex               g_mutex;

	counted_ptr<ImageManager>       m_manager;
	ImageKey						m_key;

	glm::i16vec2                    m_size{0, 0};
	uint8_t                         m_channels{0};

	bool                            m_hasAlpha{0};
	bool                            m_isLoaded{0};
	bool                            m_ownsTexture{0};

	uint32_t                        m_texture{0};
	mutable std::atomic<int>        m_refCount{1};
};

#endif // IMAGE_H
