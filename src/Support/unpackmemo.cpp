#include "unpackmemo.h"
#include "gltf_stl_accessor.h"
#include "Sprite/imagemanager.h"
#include "Support/getuniquecountedarray.h"
#include "Sprite/imagetexturecoordinates.h"

UnpackMemo::UnpackMemo(std::string const& documentFilePath) : documentFilePath(documentFilePath) {}
UnpackMemo::~UnpackMemo() {}

counted_ptr<Image> UnpackMemo::UnpackImage(ImageManager * manager, UnpackMemo & memo, Sprites::Document const& doc, fx::gltf::Material::Texture const& _texture)
{
	if((uint32_t)_texture.index >= doc.textures.size())
		return {};

	auto & texture = doc.textures[_texture.index];

	if((uint32_t)texture.source >= doc.images.size())
		return {};

	if((uint32_t)texture.texCoords >= doc.texCoords.size())
		return {};

	ImageKey key(memo.documentFilePath, texture.source);

	return manager->GetImage(memo, doc, texture.source, texture.texCoords);
}


counted_ptr<ImageTextureCoordinates> UnpackMemo::GetTexCoords(Sprites::Document const& doc, int i)
{
	if(m_texCoords.size() != doc.texCoords.size())
		m_texCoords.resize(doc.texCoords.size());

	if((uint32_t)i < m_texCoords.size() && !m_texCoords[i].empty())
		return m_texCoords[i];

	return (m_texCoords[i] = ImageTextureCoordinates::Factory(doc, i, *this));
}

template<int n, typename T, glm::qualifier Q>
ConstSizedArray<glm::vec<n, T, Q>>  LoadAccessorArray(Sprites::Document const& doc, uint32_t i, glm::vec<n, T, Q>)
{
	fx::gltf::stdAccessor<T, n, glm::vec<n, T, Q> > accessor(doc, i);

	CountedSizedArray<glm::vec<n, T, Q>> r(accessor.size());

	for(auto i = 0u; i < r.size(); ++i)
		r[i] = accessor[i];

	return r;
}

template<typename T>
ConstSizedArray<T> UnpackMemo::GetAccessor(Sprites::Document const& doc, int i, std::vector<ConstSizedArray<T>> UnpackMemo::*array)
{
	if(i < 0) return {};

	if((this->*array).size() != doc.accessors.size())
		(this->*array).resize(doc.accessors.size());

	if((uint32_t)i < (this->*array).size() && !(this->*array)[i].empty())
		return (this->*array)[i];

	return ((this->*array)[i] = MakeUnique(LoadAccessorArray(doc, i, T())));
}

ConstSizedArray<glm::i16vec4> UnpackMemo::GetAccessor_i16vec4(Sprites::Document const& doc, int i)
{
	return GetAccessor<glm::i16vec4>(doc, i, &UnpackMemo::m_i16vec4Accessors);
}

ConstSizedArray<glm::u16vec4> UnpackMemo::GetAccessor_u16vec4(Sprites::Document const& doc, int i)
{
	return GetAccessor<glm::u16vec4>(doc, i, &UnpackMemo::m_u16vec4Accessors);
}

ConstSizedArray<glm::i16vec2> UnpackMemo::GetAccessor_i16vec2(Sprites::Document const& doc, int i)
{
	return GetAccessor<glm::i16vec2>(doc, i, &UnpackMemo::m_i16vec2Accessors);
}

