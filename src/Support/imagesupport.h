#ifndef IMAGESUPPORT_H
#define IMAGESUPPORT_H
#include "Support/counted_string.h"
#include "Support/countedsizedarray.hpp"
#include <glm/gtc/type_precision.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <memory>

#undef LoadImage

#define USE_BASISU 0

class GLViewWidget;

#if USE_BASISU
namespace basisu
{
class image;
}
#else
class QImage;
#endif

namespace IO
{
struct std_free { void operator()(void * it) { std::free(it); } };

typedef std::unique_ptr<uint8_t[], std_free> ImageData;

	enum class Format : int8_t
	{
		Unknown = -1,
		None,
		BMP,
		JPEG,
		PNG,
	//	TGA,
	};

	Format GetFormat(const char *, int length = -1);

	struct Image
	{
		ImageData    image{nullptr};
		glm::i16vec2 size{0, 0};
		uint32_t     format{};
		uint32_t     internalFormat{};
		uint32_t     type{};
	};

	IO::Image LoadImageOrSprite(const char * path);
	IO::Image LoadImage(const char * path);
//	void SaveImage(const char * path, uint8_t * data, glm::i16vec2 size, int channels);

	void UploadImage(GLViewWidget * gl, uint32_t * texture, uint8_t * data, glm::i16vec2 size, uint32_t internal_format, uint32_t format, uint32_t type);
	void DownloadImage(GLViewWidget *, Image *,  uint32_t, int internalFormat);
#if USE_BASISU
	void DownloadImage(GLViewWidget *, basisu::image &,  uint32_t);
#else
	void DownloadImage(GLViewWidget *, QImage *,  uint32_t);
#endif

	glm::i16vec4 GetSprite(uint8_t * data, glm::i16vec2 size, int channels, glm::i16vec2 tl, uint32_t color);
	glm::i16vec4 GetCrop  (const uint8_t * data, glm::i16vec2 size, int channels, glm::i16vec4 aabb, uint32_t greenMask, uint32_t greenScreen);

	CountedSizedArray<glm::i16vec4> GetSprites(uint8_t * data, glm::i16vec2 size, int channels);
	CountedSizedArray<glm::i16vec4> GetCrop(uint8_t * data, glm::i16vec2 size, int channels, CountedSizedArray<glm::i16vec4> sprites);
	CountedSizedArray<glm::u16vec4> NormalizeCrop(CountedSizedArray<glm::i16vec4> sprites, glm::i16vec2 size);

	bool CheckDynamics(std::string & error, float & size_ratio, CountedSizedArray<glm::i16vec4> A, CountedSizedArray<glm::i16vec4> B);
};


#endif // IMAGE_H
