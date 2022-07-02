#include "spritejson.h"
#include <fstream>
#include <climits>

#define ReadRequiredField fx::gltf::detail::ReadRequiredField
#define ReadOptionalField fx::gltf::detail::ReadOptionalField
#define WriteQuick        fx::gltf::detail::WriteField
#define WriteRequiredField fx::gltf::detail::WriteRequiredField

namespace fx {
namespace gltf {
void from_json(nlohmann::json const & json, Asset & db);
void from_json(nlohmann::json const & json, Buffer & db);
void from_json(nlohmann::json const & json, Accessor & db);
void from_json(nlohmann::json const & json, BufferView & db);
void from_json(nlohmann::json const & json, Image & db);
void from_json(nlohmann::json const & json, Texture & db);
void from_json(nlohmann::json const & json, Sampler & db);
void from_json(nlohmann::json const & json, Material & db);

void to_json(nlohmann::json & json, Asset const& db);
void to_json(nlohmann::json & json, Buffer const& db);
void to_json(nlohmann::json & json, Accessor const& db);
void to_json(nlohmann::json & json, BufferView const& db);
void to_json(nlohmann::json & json, Image const& db);
void to_json(nlohmann::json & json, Texture const& db);
void to_json(nlohmann::json & json, Sampler const& db);
void to_json(nlohmann::json & json, Material const& db);
}}

namespace Sprites
{


namespace detail
{

inline Document Create(nlohmann::json const & json, DataContext const & dataContext, bool skip_buffers)
{
	Document document = json;

	if(skip_buffers)
		return document;

	LoadBuffers(document.buffers, dataContext);
	return document;
}

inline void Save(Document const& document, std::string const& documentFilePath, bool useBinaryFormat)
{
	nlohmann::json json = document;

	SaveInternal(std::move(json), document.buffers, documentFilePath, useBinaryFormat, SPRHeaderMagic);
}

};

inline void from_json(nlohmann::json const & json, Animation & db)
{
	ReadRequiredField("name",   json, db.name);
	ReadRequiredField("frames", json, db.frames);
	ReadRequiredField("fps",    json, db.fps);

	detail::ReadExtensionsAndExtras(json, db.extensionsAndExtras);
}

inline void to_json(nlohmann::json & json, Animation const& db)
{
	WriteQuick("name",   json, db.name);
	WriteQuick("frames", json, db.frames);
	WriteQuick("fps",    json, db.fps, 29.97f);

	detail::WriteExtensions(json, db.extensionsAndExtras);
}

inline void from_json(nlohmann::json const & json, TexCoords & db)
{
	ReadRequiredField("sprites",			json, db.sprites);
	ReadRequiredField("cropped",			json, db.cropped);
	ReadRequiredField("normalizedSprites",  json, db.normalizedSprites);
	ReadRequiredField("normalizedCrop",		json, db.normalizedCrop);
}

inline void to_json(nlohmann::json & json, TexCoords const& db)
{
	WriteRequiredField("sprites",			 json, db.sprites);
	WriteRequiredField("cropped",			 json, db.cropped);
	WriteRequiredField("normalizedSprites",  json, db.normalizedSprites);
	WriteRequiredField("normalizedCrop",	 json, db.normalizedCrop);
}

inline void from_json(nlohmann::json const & json, Sprite::Frame & db)
{
	ReadOptionalField("attachments", json, db.attachments);
	ReadRequiredField("count",		 json, db.count);
	ReadRequiredField("AABB",        json, db.AABB);
	ReadRequiredField("crop",        json, db.crop);
	ReadRequiredField("texCoord0",   json, db.texCoord0);
	ReadRequiredField("texCoord1",   json, db.texCoord1);
}

inline void to_json(nlohmann::json & json, Sprite::Frame const& db)
{
	fx::gltf::detail::WriteField("attachments", json, db.attachments, -1);
	WriteRequiredField("count",       json, db.count);
	WriteRequiredField("AABB",        json, db.AABB);
	WriteRequiredField("crop",        json, db.crop);
	WriteRequiredField("texCoord0",   json, db.texCoord0);
	WriteRequiredField("texCoord1",   json, db.texCoord1);
}

inline void from_json(nlohmann::json const & json, Sprite & db)
{
	ReadRequiredField("name",         json, db.name);
	ReadRequiredField("material",     json, db.material);

	ReadRequiredField("frames",       json, db.frames);

	ReadOptionalField("attachments",  json, db.attachments);
	ReadOptionalField("animations",   json, db.animations);

	detail::ReadExtensionsAndExtras(json, db.extensionsAndExtras);
}

void to_json(nlohmann::json & json, Sprite const& db)
{
	WriteQuick("name",         json, db.name);
	WriteQuick("material",     json, db.material, -1);

	WriteQuick("frames",       json, db.frames);

	WriteQuick("attachments",  json, db.attachments);
	WriteQuick("animations",   json, db.animations);

	detail::WriteExtensions(json, db.extensionsAndExtras);
}

void from_json(nlohmann::json const & json, Document & db)
{
	ReadRequiredField("asset",          json, db.asset);

	if(std::stof(db.asset.version) <= 2.0)
	{
		throw std::runtime_error("asset.version too low for this version of sprite builder.");
	}

	ReadOptionalField("sprites",        json, db.sprites);

	ReadOptionalField("accessors",      json, db.accessors);
	ReadOptionalField("buffers",        json, db.buffers);
	ReadOptionalField("bufferViews",    json, db.bufferViews);

	ReadOptionalField("materials",      json, db.materials);
	ReadOptionalField("textures",       json, db.textures);
	ReadOptionalField("images",         json, db.images);
	ReadOptionalField("samplers",       json, db.samplers);

	ReadOptionalField("texCoords",      json, db.texCoords);

	ReadOptionalField("extensionsUsed", json, db.extensionsUsed);
	ReadOptionalField("extensionsRequired", json, db.extensionsRequired);

	detail::ReadExtensionsAndExtras(json, db.extensionsAndExtras);
}

void to_json(nlohmann::json & json, Document const& db)
{
	auto asset = db.asset;
	asset.version = "2.01";

	WriteQuick("asset",          json, asset);
	WriteQuick("sprites",        json, db.sprites);

	WriteQuick("accessors",      json, db.accessors);
	WriteQuick("buffers",        json, db.buffers);
	WriteQuick("bufferViews",    json, db.bufferViews);

	WriteQuick("materials",      json, db.materials);
	WriteQuick("textures",       json, db.textures);
	WriteQuick("images",         json, db.images);
	WriteQuick("samplers",       json, db.samplers);

	WriteQuick("texCoords",      json, db.texCoords);

	WriteQuick("extensionsUsed", json, db.extensionsUsed);
	WriteQuick("extensionsRequired", json, db.extensionsRequired);

	detail::WriteExtensions(json, db.extensionsAndExtras);
}

Document LoadFromBinary(std::vector<uint8_t> binary, std::string const & documentFilePath, bool skip_buffers, ReadQuotas const & readQuotas)
{
	detail::SPRHeader header;
	std::memcpy(&header, &binary[0], detail::HeaderSize);

	if (header.magic != detail::SPRHeaderMagic ||
	    header.jsonHeader.chunkType != detail::SPRChunkJSON ||
	    header.jsonHeader.chunkLength + detail::HeaderSize > header.length)
	{
		throw invalid_document("Invalid GLB header");
	}

	return detail::Create(
		nlohmann::json::parse({ &binary[detail::HeaderSize], header.jsonHeader.chunkLength }),
		{ detail::GetDocumentRootPath(documentFilePath), readQuotas, &binary, header.jsonHeader.chunkLength + detail::HeaderSize },
			skip_buffers);
}

Document LoadFromText(std::string const & documentFilePath, bool skip_buffers, ReadQuotas const & readQuotas)
{
	nlohmann::json json;
	{
		std::ifstream file(documentFilePath);
		if (!file.is_open())
		{
			throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
		}

		file >> json;
	}

	return detail::Create(json, { detail::GetDocumentRootPath(documentFilePath), readQuotas }, skip_buffers);
}

Document LoadFromBinary(std::string const & documentFilePath, bool skip_buffers, ReadQuotas const & readQuotas)
{
	std::vector<uint8_t> binary{};
	{
		std::ifstream file(documentFilePath, std::ios::binary);

		if (!file.is_open())
		{
			throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory), documentFilePath);
		}

		file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );

		const std::size_t fileSize = detail::GetFileSize(file);
		if (fileSize < detail::HeaderSize)
		{
			throw invalid_document("Invalid Sprite file");
		}

		if (fileSize > readQuotas.MaxFileSize)
		{
			throw invalid_document("Quota exceeded : file size > MaxFileSize");
		}

		binary.resize(fileSize);
		file.read(reinterpret_cast<char *>(&binary[0]), fileSize);
	}

	return Sprites::LoadFromBinary(binary, documentFilePath, skip_buffers, readQuotas);
}

Document & LoadExternalBuffers(Document & document, std::string const & documentFilePath, ReadQuotas const & readQuotas)
{
	detail::LoadBuffers(document.buffers, { detail::GetDocumentRootPath(documentFilePath), readQuotas });

	return document;
}

void Save(Document const & document, std::string documentFilePath, bool useBinaryFormat)
{
	detail::ValidateBuffers(document.buffers, useBinaryFormat);
	detail::Save(document, documentFilePath, useBinaryFormat);
}
#if 0
void AtlasFile::Load()
{
	std::ifstream file(path, std::ios::in);

	if (!file.is_open())
	{
		throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory), path);
	}

	file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );

	std::string line;
	std::getline(file, line);

	if(line.find_first_of("textures: ") != 0)
		throw invalid_document("Atlas file improperly formatted, must start with 'textures: .*\\n'");

	imageFile = line.substr(11, std::string::npos);

	Frame frame;
	while(!file.eof())
	{
		std::getline(file, frame.image, '\t');
		std::getline(file, line, '\t');
		frame.bounding_box[0] = std::stoi(line);
		std::getline(file, line, '\t');
		frame.bounding_box[1] = std::stoi(line);
		std::getline(file, line, '\t');
		frame.bounding_box[2] = std::stoi(line);
		std::getline(file, line, '\t');
		frame.bounding_box[3] = std::stoi(line);
		std::getline(file, line, '\t');

		frame.crop_box[0] = std::stoi(line);
		std::getline(file, line, '\t');
		frame.crop_box[1] = std::stoi(line);
		std::getline(file, line, '\t');
		frame.crop_box[2] = std::stoi(line);
		std::getline(file, line, '\t');
		frame.crop_box[3] = std::stoi(line);

		std::getline(file, line, '\n');
		frame.rotated = (!line.empty() && line.front() == 'r');

		frames.push_back(frame);
	}

	file.close();
}
#endif

}
