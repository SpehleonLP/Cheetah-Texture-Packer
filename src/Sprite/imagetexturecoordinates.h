#ifndef IMAGETEXTURECOORDINATES_H
#define IMAGETEXTURECOORDINATES_H
#include "Support/shared_array.hpp"
#include "Support/counted_ptr.hpp"
#include <glm/gtc/type_precision.hpp>

namespace IO { struct Image; }
namespace Sprites { struct Document; }

struct UnpackMemo;
struct PackMemo;

class ImageTextureCoordinates
{
public:
typedef immutable_array<glm::i16vec4> i16vec4array;
typedef immutable_array<glm::u16vec4> u16vec4array;
	enum Compatibility
	{
		None,
		CountMismatch,
		NormSizeMismatch
	};

	static counted_ptr<ImageTextureCoordinates> Factory(Sprites::Document const& doc, int texCoordId, UnpackMemo & memo);
	static counted_ptr<ImageTextureCoordinates> Factory(IO::Image const& image, i16vec4array sprites = {});
	static counted_ptr<ImageTextureCoordinates> Factory(std::vector<counted_ptr<ImageTextureCoordinates>> const&);

	int32_t Pack(Sprites::Document & doc, PackMemo & memo) const;

	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) { delete this; } }

	size_t size() const { return sprites().size(); }

	i16vec4array sprites() const { return m_sprites; };
	i16vec4array cropped() const { return m_cropped; };
	u16vec4array normalizedCrop() const { return m_normalizedCrop; };
	u16vec4array normalizedPositions() const { return m_normalizedSprites; };

	bool operator==(ImageTextureCoordinates const& it) const
	{
		return m_sprites == it.m_sprites
			&& m_cropped == it.m_cropped
			&& m_normalizedCrop == it.m_normalizedCrop
			&& m_normalizedSprites == it.m_normalizedSprites;
	}

	Compatibility IsCompatible(ImageTextureCoordinates const * it) const
	{
		if(it->size() != size())
			return CountMismatch;

	//	if(m_normalizedSprites != it->m_normalizedSprites)
	//		return NormSizeMismatch;

		return None;
	}

private:
	static counted_ptr<ImageTextureCoordinates> Factory(i16vec4array const& sprites, i16vec4array const& crop, u16vec4array const& normalized, u16vec4array const& positions);

	ImageTextureCoordinates(i16vec4array const&sprites, i16vec4array const&crop, u16vec4array const&normalized, u16vec4array const&positions);
	~ImageTextureCoordinates();

	mutable std::atomic<int>        m_refCount{1};

	i16vec4array m_sprites;
	i16vec4array m_cropped;
	u16vec4array m_normalizedSprites;
	u16vec4array m_normalizedCrop;

	static std::vector<counted_ptr<ImageTextureCoordinates>> g_database;
};

#endif // IMAGETEXTURECOORDINATES_H
