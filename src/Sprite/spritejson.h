#ifndef SPRITEJSON_H
#define SPRITEJSON_H
//#include <fx/extensions/msft_texture_dds.cpp>
#include <fx/gltf.h>

namespace Sprites
{

typedef fx::gltf::NeverEmpty  NeverEmpty;
typedef fx::gltf::Accessor    Accessor;
typedef fx::gltf::Buffer      Buffer;
typedef fx::gltf::BufferView  BufferView;
typedef fx::gltf::Image       Image;
typedef fx::gltf::Texture     Texture;
typedef fx::gltf::Sampler     Sampler;
typedef fx::gltf::Material    Material;
typedef fx::gltf::ReadQuotas  ReadQuotas;
typedef fx::gltf::DataContext DataContext;
typedef fx::gltf::Asset       Asset;


typedef fx::gltf::invalid_gltf_document invalid_document;

namespace detail
{
typedef fx::gltf::detail::ChunkHeader ChunkHeader;
typedef fx::gltf::detail::GLBHeader   SPRHeader;

constexpr uint32_t DefaultMaxBufferCount = 8;
constexpr uint32_t DefaultMaxMemoryAllocation = 1024 * 1024 * 1024;
constexpr std::size_t HeaderSize{ sizeof(SPRHeader) };
constexpr std::size_t ChunkHeaderSize{ sizeof(ChunkHeader) };
constexpr uint32_t SPRHeaderMagic = 0x32334C46u;
constexpr uint32_t SPRChunkJSON = 0x4e4f534au;
constexpr uint32_t SPRChunkBIN = 0x004e4942u;

constexpr char const * const MimetypeApplicationOctet = "data:application/octet-stream;base64";
constexpr char const * const MimetypeImagePNG = "data:image/png;base64";
constexpr char const * const MimetypeImageJPG = "data:image/jpeg;base64";

constexpr auto ReadExtensionsAndExtras = fx::gltf::detail::ReadExtensionsAndExtras;
constexpr auto WriteExtensions         = fx::gltf::detail::WriteExtensions;

constexpr auto GetDocumentRootPath = fx::gltf::detail::GetDocumentRootPath;
constexpr auto GetFileSize         = fx::gltf::detail::GetFileSize;
constexpr auto LoadBuffers         = fx::gltf::detail::LoadBuffers;
constexpr auto ValidateBuffers     = fx::gltf::detail::ValidateBuffers;
constexpr auto SaveInternal        = fx::gltf::detail::Save;

};

struct Animation
{
	std::string           name;
	std::vector<uint16_t> frames;
	float                 fps{20};
	uint16_t              loop_start{0};
	uint16_t              loop_end{0};

	nlohmann::json extensionsAndExtras{};
};

struct TexCoords
{
	int32_t sprites{-1};
	int32_t cropped{-1};
	int32_t normalizedSprites{-1};
	int32_t normalizedCrop{-1};
};

struct Sprite : NeverEmpty
{
	struct Frame : NeverEmpty
	{
		int32_t					 attachments{-1};
		int32_t					 count{};

//original position on the sprite sheet
		int32_t					 AABB{-1};
		int32_t					 crop{-1};
		int32_t					 texCoord0{-1};
		int32_t					 texCoord1{-1};
	};

	std::string              name;

	int32_t                  material{-1};

	Frame			         frames;
	std::vector<std::string> attachments;
	std::vector<Animation>   animations;

	nlohmann::json extensionsAndExtras{};
};

struct Document : public fx::gltf::DocumentBase
{
	Asset                   asset;

	std::vector<Sprite>     sprites{};
	std::vector<Material>   materials{};
	std::vector<TexCoords>  texCoords{};

	std::vector<std::string> extensionsUsed{};
	std::vector<std::string> extensionsRequired{};

	nlohmann::json extensionsAndExtras{};
};

Document Import(std::ifstream &&, std::string const& documentFilePath);

Document LoadFromBinary(std::vector<uint8_t> binary, std::string const & documentFilePath, bool skip_buffers = false, ReadQuotas const & readQuotas = {});
Document LoadFromText(std::string const & documentFilePath, bool skip_buffers = false, ReadQuotas const & readQuotas = {});
Document LoadFromBinary(std::string const & documentFilePath, bool skip_buffers = false, ReadQuotas const & readQuotas = {});
Document & LoadExternalBuffers(Document & document, std::string const & documentFilePath, ReadQuotas const & readQuotas = {});
void Save(Document const & document, std::string documentFilePath, bool useBinaryFormat);

};

#endif // SPRITEJSON_H
