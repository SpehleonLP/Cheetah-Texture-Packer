#ifndef IMAGETEXTURECOORDINATES_H
#define IMAGETEXTURECOORDINATES_H
#include "Support/countedsizedarray.hpp"
#include <glm/gtc/type_precision.hpp>

namespace IO { struct Image; }
namespace Sprites { struct Document; }

struct UnpackMemo;
struct PackMemo;

class ImageTextureCoordinates
{
public:
typedef ConstSizedArray<glm::i16vec4> i16vec4array;
typedef ConstSizedArray<glm::u16vec4> u16vec4array;
	enum Compatibility
	{
		None,
		CountMismatch,
		NormSizeMismatch
	};

	static counted_ptr<ImageTextureCoordinates> Factory(Sprites::Document const& doc, int texCoordId, UnpackMemo & memo);
	static counted_ptr<ImageTextureCoordinates> Factory(IO::Image const& image, i16vec4array sprites = {});
	static counted_ptr<ImageTextureCoordinates> Factory(std::vector<counted_ptr<ImageTextureCoordinates>> const&);

	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) { delete this; } }

	size_t size() const { return sprites().size(); }

	i16vec4array sprites() const { return m_sprites; };
	i16vec4array cropped() const { return m_cropped; };
	u16vec4array normalized() const { return m_normalized; };
	u16vec4array normalizedPositions() const { return m_normalizedPositions; };

	bool operator==(ImageTextureCoordinates const& it) const
	{
		return m_sprites == it.m_sprites
			&& m_cropped == it.m_cropped
			&& m_normalized == it.m_normalized
			&& m_normalizedPositions == it.m_normalizedPositions;
	}

	Compatibility IsCompatible(ImageTextureCoordinates const * it) const
	{
		if(it->size() != size())
			return CountMismatch;

		if(m_normalizedPositions != it->m_normalizedPositions)
			return NormSizeMismatch;

		return None;
	}

private:
	static counted_ptr<ImageTextureCoordinates> Factory(i16vec4array const& sprites, i16vec4array const& crop, u16vec4array const& normalized, u16vec4array const& positions);

	ImageTextureCoordinates(i16vec4array const&sprites, i16vec4array const&crop, u16vec4array const&normalized, u16vec4array const&positions);
	~ImageTextureCoordinates();

	mutable std::atomic<int>        m_refCount{1};

	i16vec4array m_sprites;
	i16vec4array m_cropped;
	u16vec4array m_normalized;
	u16vec4array m_normalizedPositions;

	static std::vector<counted_ptr<ImageTextureCoordinates>> g_database;
};

#endif // IMAGETEXTURECOORDINATES_H
