#ifndef IMAGETEXTURECOORDINATES_H
#define IMAGETEXTURECOORDINATES_H
#include "Support/countedsizedarray.hpp"
#include <glm/gtc/type_precision.hpp>


class ImageTextureCoordinates
{
public:
	 counted_ptr<ImageTextureCoordinates> Factory();


	ImageTextureCoordinates();

	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) { delete this; } }

	CountedSizedArray<glm::i16vec4> m_sprites;
	CountedSizedArray<glm::i16vec4> m_cropped;
	CountedSizedArray<glm::u16vec4> m_normalized;
	CountedSizedArray<glm::u16vec4> m_normalizedPositions;

private:
	mutable std::atomic<int>        m_refCount{1};

};

#endif // IMAGETEXTURECOORDINATES_H
