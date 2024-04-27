#ifndef MATERIAL_H
#define MATERIAL_H
#include "image.h"
#include <fx/gltf.h>
#include <fx/extensions/khr_materials.h>
#include "Support/counted_ptr.hpp"
#include "spritesheet.h"
#include "universal_include.h"

struct Document;
class SpriteSheet;
struct PackMemo;
struct UnpackMemo;

namespace Sprites
{
struct Document;
struct Sprite;
};

struct RenderData
{
	uint32_t first;
	uint32_t elements;

	void * offset() const  { return (void*)(first * sizeof(short)); }

	bool     center;
	short    frame;
	glm::mat4 matrix;
};

struct MaterialExtensions
{
	KHR::materials::pbrSpecularGlossiness pbrSpecularGlossiness;
	KHR::materials::clearcoat			  clearcoat;
	KHR::materials::unlit				  unlit;

#if KHR_SHEEN
	KHR::materials::sheen                 sheen;
#endif

	bool empty() const { return unlit.empty() && pbrSpecularGlossiness.empty()
#if KHR_SHEEN
		&& sheen.empty()
#endif

		&& clearcoat.empty()
		; };
};

struct Material : fx::gltf::Material
{
public:
	enum class Tex : int8_t
	{
		None = -1,
		BaseColor,
		Diffuse,
		MetallicRoughness,
		SpecularGlossiness,
		Normal,
		Occlusion,
		Emission,
		Total
	};

	enum
	{
//sprite sheet
		v_sheetCoordBegin,
		v_sheetCoordEnd = v_sheetCoordBegin + (int)Tex::Total,

		v_positions = v_sheetCoordEnd,
		v_spriteId,
		v_texCoord,
		v_indices,
		VBOc
	};

	enum class TexCoords
	{
		Total = 2
	};


typedef std::array<immutable_array<glm::u16vec4>, (int)TexCoords::Total>	TexCoord_t;
typedef std::array<int, (int)Tex::Total>									TexIndex_t;
typedef std::array<counted_ptr<Image>, (int)Tex::Total>						ImageSlot_t;

	static int PackDocument(Material * mat, Sprites::Document & doc, PackMemo & mapping, glm::ivec4 & frame);

	Material();
	Material(ImageManager * manager, Sprites::Sprite const& spr, Sprites::Document const& doc, UnpackMemo & memo);
	~Material();

	MaterialExtensions ext;

	ImageSlot_t						     image_slots;
	counted_ptr<ImageTextureCoordinates>  m_texCoords;

	void Clear(GLViewWidget * gl);

	bool isUnlit() const { return !ext.unlit.is_empty; }
	bool isSpecular() const { return !ext.pbrSpecularGlossiness.is_empty; }

	void RenderObjectSheet(GLViewWidget *, int frame = -1);
	void RenderSpriteSheet(GLViewWidget *, Tex image_slot, int frame = -1);

	void Render(GLViewWidget * gl, Material::Tex texture, int frame, int outline);

	std::string IsImageCompatible(Material::Tex, counted_ptr<Image>);
	void SetImage(counted_ptr<Image> image, counted_ptr<Image> * slot);

	TexIndex_t GetTextureCoordinates(counted_ptr<Image> image);
	TexIndex_t GetTextureCoordinates(TexCoord_t & coord);

	inline TexIndex_t GetTextureCoordinates() { TexCoord_t c; return GetTextureCoordinates(c); }

	inline int & TexCoord(Tex tex)
	{
		static int x{};

		switch(tex)
		{
		case Tex::BaseColor:			return pbrMetallicRoughness.baseColorTexture.texCoord;
		case Tex::Diffuse:				return ext.pbrSpecularGlossiness.diffuseTexture.texCoord;
		case Tex::MetallicRoughness:	return pbrMetallicRoughness.metallicRoughnessTexture.texCoord;
		case Tex::SpecularGlossiness:   return ext.pbrSpecularGlossiness.specularGlossinessTexture.texCoord;
		case Tex::Normal:				return normalTexture.texCoord;
		case Tex::Occlusion:			return occlusionTexture.texCoord;
		case Tex::Emission:				return emissiveTexture.texCoord;
		default:
			BreakIfDebugging();
			throw std::logic_error("Unknown material map value.");
		}

		return x;
	}

	uint32_t noFrames() const  { return m_spriteCount; }

	void Prepare(GLViewWidget*);

	glm::vec2 SheetSize() const { return m_sheetSize; }
	glm::vec2 SpriteSize(int it) const;

private:
	void CreateDefaultArrays(GLViewWidget* gl);
	void InitializeTexCoords();

	std::vector<short> CreateIdBuffer() const;
	void CreatePositionsFromNormalizedPositions(GLViewWidget * gl);

	RenderData GetRenderData(int frame);
	bool RenderSheetBackdrop(GLViewWidget * gl, RenderData const& frame);
	bool PrepareSheetBackdrop();

	struct Pair
	{
		uint16_t start;
		uint16_t length;

		uint16_t end() const { return start+length; }
	};

	bool								 m_dirty{true};

//made in render
	std::unique_ptr<SpriteSheet>    m_spriteSheet;
//made in set image
	uint32_t                        m_spriteCount{};
	glm::u16vec2                    m_sheetSize{};

//made in prepare
	uint32_t     m_vao{};
	uint32_t     m_vbo[VBOc]{};
	uint32_t     m_dirtyTextureFlags{0};

//made in prepare -> create default arrays
	immutable_array<glm::vec2>    m_normalizedPositions{};
	immutable_array<Pair>         m_spriteIndices{};
	immutable_array<Pair>         m_spriteVertices{};

private:
	immutable_array<glm::vec2>  CreateNormalizedPositions() const;

	void LoadExtensionsAndExtras();
};

#endif // MATERIAL_H
