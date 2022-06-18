#ifndef UNPACKMEMO_H
#define UNPACKMEMO_H
#include "Sprite/image.h"
#include "Sprite/spritejson.h"
#include <fx/gltf.h>
#include <glm/gtc/type_precision.hpp>
#include "Support/counted_ptr.hpp"


//normalized positions field of image = normalized sprites of material
//normalized field of image = texCoord0/1 of sprite sheet
//crop field of image can be reconstructed from normalized and image size
//sprites field of image seems to get lost in saving...

//need to get normalized positions field


struct UnpackMemo
{
typedef counted_ptr<ImageTextureCoordinates> TexCoords;

	UnpackMemo();
	~UnpackMemo();

	ConstSizedArray<glm::i16vec4> GetAccessor_i16vec4(Sprites::Document const& doc, int i);
	ConstSizedArray<glm::i16vec2> GetAccessor_i16vec2(Sprites::Document const& doc, int i);
	ConstSizedArray<glm::u16vec4> GetAccessor_u16vec4(Sprites::Document const& doc, int i);
	counted_ptr<ImageTextureCoordinates> GetTexCoords(Sprites::Document const& doc, int i);

	counted_ptr<Image> UnpackImage(GLViewWidget * gl, Sprites::Sprite const& spr, Sprites::Document const& doc, fx::gltf::Material::Texture const& texture);

	std::vector<counted_ptr<Image>> image_memo;


private:
	void UpdateFrames(Sprites::Sprite const& spr);

	template<typename T>
	ConstSizedArray<T> GetAccessor(Sprites::Document const& doc, int i, std::vector<ConstSizedArray<T>> UnpackMemo::*array);

//current accessors
	Sprites::Sprite const* current{};

	CountedSizedArray<glm::i16vec4> aabb;
	CountedSizedArray<glm::i16vec4> crop;
	CountedSizedArray<glm::u16vec4> texCoord0;
	CountedSizedArray<glm::u16vec4> texCoord1;

//previously loaded accessors
	std::vector<ConstSizedArray<glm::i16vec4>> m_i16vec4Accessors;
	std::vector<ConstSizedArray<glm::i16vec2>> m_i16vec2Accessors;
	std::vector<ConstSizedArray<glm::u16vec4>> m_u16vec4Accessors;
	std::vector<TexCoords>					   m_texCoords;
};

#endif // UNPACKMEMO_H
