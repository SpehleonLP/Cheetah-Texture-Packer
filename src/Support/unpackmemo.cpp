#include "unpackmemo.h"

UnpackMemo::UnpackMemo() {}
UnpackMemo::~UnpackMemo() {}

counted_ptr<Image> UnpackMemo::UnpackImage(GLViewWidget * gl, Sprites::Sprite const& spr, Sprites::Document const& doc, fx::gltf::Material::Texture const& texture)
{
	UpdateFrames(spr);






}

void UnpackMemo::UpdateFrames(Sprites::Sprite const& spr)
{
	static_assert(sizeof(aabb[0]) == sizeof(Sprites::Sprite::Frame::AABB));
	static_assert(sizeof(crop[0]) == sizeof(Sprites::Sprite::Frame::crop));
	static_assert(sizeof(texCoord0[0]) == sizeof(Sprites::Sprite::Frame::texCoord0));
	static_assert(sizeof(texCoord1[0]) == sizeof(Sprites::Sprite::Frame::texCoord1));

	if(current == &spr)
		return;

	current = &spr;

	aabb		= CountedSizedArray<glm::i16vec4>(spr.frames.size());
	crop		= CountedSizedArray<glm::i16vec4>(spr.frames.size());
	texCoord0	= CountedSizedArray<glm::u16vec4>(spr.frames.size());
	texCoord1	= CountedSizedArray<glm::u16vec4>(spr.frames.size());

	for(uint32_t i = 0; i < spr.frames.size(); ++i)
	{
		memcpy(&aabb[i][0], &spr.frames[i].AABB[0], sizeof(aabb[0]));
		memcpy(&crop[i][0], &spr.frames[i].crop[0], sizeof(crop[0]));
		memcpy(&texCoord0[i][0], &spr.frames[i].texCoord0[0], sizeof(texCoord0[0]));
		memcpy(&texCoord1[i][0], &spr.frames[i].texCoord1[0], sizeof(texCoord1[0]));
	}

	aabb	  = MakeUnique(aabb);
	crop	  = MakeUnique(crop);
	texCoord0 = MakeUnique(texCoord0);
	texCoord1 = MakeUnique(texCoord1);



}
