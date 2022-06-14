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
	UnpackMemo();
	~UnpackMemo();

	counted_ptr<Image> UnpackImage(GLViewWidget * gl, Sprites::Sprite const& spr, Sprites::Document const& doc, fx::gltf::Material::Texture const& texture);

	std::vector<counted_ptr<Image>> image_memo;


private:
	void UpdateFrames(Sprites::Sprite const& spr);

//current accessors
	Sprites::Sprite const* current{};

	CountedSizedArray<glm::i16vec4> aabb;
	CountedSizedArray<glm::i16vec4> crop;
	CountedSizedArray<glm::u16vec4> texCoord0;
	CountedSizedArray<glm::u16vec4> texCoord1;

//previously loaded accessors
	std::vector<CountedSizedArray<glm::i16vec4>> m_i16vec4Accessors;
	std::vector<CountedSizedArray<glm::u16vec4>> m_u16vec4Accessors;

	CountedSizedArray<glm::u16vec4> MakeUnique(CountedSizedArray<glm::u16vec4> const& in)
	{
		for(auto i = 0u; i < m_u16vec4Accessors.size(); ++i)
		{
			if(m_u16vec4Accessors[i].CanMerge(in))
			{
				return m_u16vec4Accessors[i];
			}
		}

		return in;
	}

	CountedSizedArray<glm::i16vec4> MakeUnique(CountedSizedArray<glm::i16vec4> const& in)
	{
		for(auto i = 0u; i < m_i16vec4Accessors.size(); ++i)
		{
			if(m_i16vec4Accessors[i].CanMerge(in))
			{
				return m_i16vec4Accessors[i];
			}
		}

		return in;
	}
};

#endif // UNPACKMEMO_H
