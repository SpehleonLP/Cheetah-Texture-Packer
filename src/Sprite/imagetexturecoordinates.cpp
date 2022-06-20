#include "imagetexturecoordinates.h"
#include "Support/imagesupport.h"
#include "Support/getuniquecountedarray.h"
#include "Support/qt_to_gl.h"
#include "Support/unpackmemo.h"
#include "Support/packaccessor.h"
#include <mutex>

std::vector<counted_ptr<ImageTextureCoordinates>> ImageTextureCoordinates::g_database;

counted_ptr<ImageTextureCoordinates> ImageTextureCoordinates::Factory(Sprites::Document const& doc, int i, UnpackMemo & memo)
{
	return Factory(
		memo.GetAccessor_i16vec4(doc, doc.texCoords[i].sprites),
		memo.GetAccessor_i16vec4(doc, doc.texCoords[i].cropped),
		memo.GetAccessor_u16vec4(doc, doc.texCoords[i].normalizedSprites),
		memo.GetAccessor_u16vec4(doc, doc.texCoords[i].normalizedCrop));
}


counted_ptr<ImageTextureCoordinates> ImageTextureCoordinates::Factory(IO::Image const& image, i16vec4array sprites)
{
	i16vec4array cropped;
	u16vec4array normalized;
	u16vec4array normalizedPositions;

	auto channels  = Qt_to_Gl::GetChannelsFromFormat(image.format);

	if(sprites.empty())
		sprites    = IO::GetSprites(&image.image[0], image.bytes, image.size, channels);

	cropped    = IO::GetCrop(&image.image[0], image.bytes, image.size, channels, sprites);
	normalized = IO::NormalizeCrop(cropped, image.size);

	normalizedPositions = IO::NormalizeCrop(sprites, image.size);

	return Factory(sprites, cropped, normalized, normalizedPositions);
}

counted_ptr<ImageTextureCoordinates> ImageTextureCoordinates::Factory(i16vec4array const& sprites, i16vec4array const& crop, u16vec4array const& normalized, u16vec4array const& positions)
{
	auto ptr = CountedWrap(new ImageTextureCoordinates(MakeUnique(sprites), MakeUnique(crop), MakeUnique(normalized), MakeUnique(positions)));

	std::lock_guard<std::mutex> lock(CountedDBBase::g_mutex);

	for(auto i = 0u; i < g_database.size(); ++i)
	{
		if(*g_database[i] == *ptr)
			return g_database[i];
	}

	g_database.push_back(ptr);
	return ptr;
}

ImageTextureCoordinates::ImageTextureCoordinates(i16vec4array const& sprites, i16vec4array const& crop, u16vec4array const& normalized, u16vec4array const& positions) :
	m_sprites(sprites),
	m_cropped(crop),
	m_normalizedSprites(positions),
	m_normalizedCrop(normalized)
{
}

ImageTextureCoordinates::~ImageTextureCoordinates()
{
}

int32_t ImageTextureCoordinates::Pack(Sprites::Document & doc, PackMemo & memo) const
{
	int32_t id = memo.GetId(this);

	if(id >= 0)
		return id;

	id = doc.texCoords.size();
	memo.SetId(this, id);

	Sprites::TexCoords buffer;

	buffer. sprites				= memo.PackAccessor(m_sprites);
	buffer. cropped				= memo.PackAccessor(m_cropped);
	buffer. normalizedSprites	= memo.PackAccessor(m_normalizedSprites, true);
	buffer. normalizedCrop		= memo.PackAccessor(m_normalizedCrop, true);

	doc.texCoords.push_back(buffer);
	return id;

}
