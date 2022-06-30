#include "material.h"
#include "imagetexturecoordinates.h"
#include "src/widgets/glviewwidget.h"
#include "src/Support/vectoroperations.hpp"
#include "Shaders/defaultvaos.h"
#include "Shaders/gltfmetallicroughness.h"
#include "Shaders/transparencyshader.h"
#include "Shaders/unlitshader.h"
#include "Support/packaccessor.h"
#include "Support/unpackmemo.h"
#include <glm/gtc/matrix_transform.hpp>
#include "spritesheet.h"
#include "spritejson.h"
#include <iostream>

#define UNUSED(x) (void)x;


Material::Material()
{
	pbrMetallicRoughness.baseColorTexture.texCoord = 2 + (int)Tex::BaseColor;
	ext.pbrSpecularGlossiness.diffuseTexture.texCoord  = 2 + (int)Tex::Diffuse;
	pbrMetallicRoughness.metallicRoughnessTexture.texCoord = 2 + (int)Tex::MetallicRoughness;
	ext.pbrSpecularGlossiness.specularGlossinessTexture.texCoord = 2 + (int)Tex::SpecularGlossiness;
	normalTexture.texCoord    = 2 + (int)Tex::Normal;
	occlusionTexture.texCoord = 2 + (int)Tex::Occlusion;
	emissiveTexture.texCoord  = 2 + (int)Tex::Emission;
}

Material::Material(ImageManager * manager, Sprites::Sprite const& spr, Sprites::Document const& doc, UnpackMemo & memo) :
	Material()
{
	*static_cast<fx::gltf::Material*>(this) = doc.materials[spr.material];

	LoadExtensionsAndExtras();

#define UnpackImage(TEXTURE, ST) \
	SetImage(memo.UnpackImage(manager, memo, doc, TEXTURE), &image_slots[(int)ST]);

	UnpackImage(pbrMetallicRoughness.baseColorTexture, Tex::BaseColor);
	UnpackImage(ext.pbrSpecularGlossiness.diffuseTexture, Tex::Diffuse);
	UnpackImage(pbrMetallicRoughness.metallicRoughnessTexture, Tex::MetallicRoughness);
	UnpackImage(ext.pbrSpecularGlossiness.specularGlossinessTexture, Tex::SpecularGlossiness);
	UnpackImage(normalTexture, Tex::Normal);
	UnpackImage(occlusionTexture, Tex::Occlusion);
	UnpackImage(emissiveTexture, Tex::Emission);
}

Material::~Material() {}

void Material::Clear(GLViewWidget * gl)
{
	if(m_spriteSheet)
		m_spriteSheet->Clear(gl);

	if(m_vao)
	{
		_gl glDeleteBuffers(VBOc, m_vbo);
		_gl glDeleteVertexArrays(1, &m_vao);

		memset(m_vbo, 0, sizeof(m_vbo));
		m_vao = 0;
	}
}

void Material::SetImage(counted_ptr<Image> image, counted_ptr<Image> * slot)
{
	if(image == nullptr)
		return;

	int tex = (slot - &image_slots[0]);

	if(!(0 <= tex && tex < (int)Tex::Total))
	{
		throw std::runtime_error("bad texture pointer...");
	}

	std::string error = IsImageCompatible((Tex)tex, image);

	if(!error.empty())
		throw std::logic_error(error);

	if(image == image_slots[tex])
		return;

	image_slots[tex] = image;
	m_dirtyTextureFlags |= (1 << tex);

	std::vector<counted_ptr<ImageTextureCoordinates>> coords;

	for(int i = 0; i < (int)Tex::Total; ++i)
	{
		if(image_slots[i])
			coords.push_back(image_slots[i]->m_texCoords);
	}

	counted_ptr<ImageTextureCoordinates> texCoord = ImageTextureCoordinates::Factory(coords);

	if(texCoord != m_texCoords)
	{
		m_dirty = true;
		m_texCoords = texCoord;

		for(int i = 0; i < (int)Tex::Total; ++i)
		{
			if(image_slots[i])
			{
				m_sheetSize   = image_slots[i]->GetSize();
				break;
			}
		}
	}

	m_spriteCount = m_texCoords? m_texCoords->size() : 0;
}

std::string Material::IsImageCompatible(Material::Tex tex, counted_ptr<Image> image)
{
	try
	{
		UNUSED(tex);

		if(image == nullptr)
			return {};

		image->LoadFromFile();

		GetTextureCoordinates(image);

	//first one so anything goes
		for(int i = 0; i < (int)Tex::Total; ++i)
		{
			if(tex == (Tex)i)
				continue;

			if(image_slots[i].empty()) continue;

			switch(image_slots[i]->m_texCoords->IsCompatible(image->m_texCoords.get()))
			{
			case ImageTextureCoordinates::Compatibility::CountMismatch:
				return "number of sprites in image does not match number in material.";
			case ImageTextureCoordinates::Compatibility::NormSizeMismatch:
				return "sprites in image do not properly align with sprites in material.";
			default:
				break;
			}

		}
	}
	catch(std::exception & e)
	{
		return e.what();
	}

	return {};
}

template<typename T>
void UploadData(GLViewWidget* gl, GLenum type, uint32_t vbo, std::vector<T> const& array)
{
	_gl glBindBuffer(type, vbo); DEBUG_GL
	_gl glBufferData(type, array.size() * sizeof(array[0]), array.data(), GL_DYNAMIC_DRAW); DEBUG_GL
};

template<typename T>
void UploadData(GLViewWidget* gl, GLenum type, uint32_t vbo, ConstSizedArray<T> array)
{
	_gl glBindBuffer(type, vbo); DEBUG_GL
	_gl glBufferData(type, array.size() * sizeof(array[0]), array.data(), GL_DYNAMIC_DRAW); DEBUG_GL
};

void Material::CreateDefaultArrays(GLViewWidget* gl)
{
	if(!m_normalizedPositions.empty() || m_spriteCount == 0)
		return;

	_gl glBindVertexArray(m_vao); DEBUG_GL

	m_normalizedPositions = CountedSizedArray<glm::vec2>(m_spriteCount * 4);
	m_spriteIndices       = CountedSizedArray<Pair>(m_spriteCount);
	m_spriteVertices      = CountedSizedArray<Pair>(m_spriteCount);

	for(uint32_t i = 0; i < m_spriteCount; ++i)
		m_spriteIndices[i] = {(uint16_t)(i*6), 6};

	for(uint32_t i = 0; i < m_spriteCount; ++i)
		m_spriteVertices[i] = {(uint16_t)(i*4), 4};

	auto m_sprites = m_texCoords->sprites();
	auto m_crop   = m_texCoords->sprites();

	for(uint32_t i = 0; i < m_spriteCount; ++i)
	{
#if 1
		glm::vec2 center = SpriteSheet::GetCenter(m_sprites[i]);
		glm::vec4 sprite = glm::vec4(m_sprites[i]) - glm::vec4(center, center);
		glm::vec4 crop   = glm::vec4(m_crop   [i]) - glm::vec4(center, center);
		glm::vec4 result = crop / glm::abs(sprite);

		m_normalizedPositions[i*4+0] = glm::vec2(result.x, result.y);
		m_normalizedPositions[i*4+1] = glm::vec2(result.z, result.y);
		m_normalizedPositions[i*4+2] = glm::vec2(result.z, result.w);
		m_normalizedPositions[i*4+3] = glm::vec2(result.x, result.w);
#else
		m_normalizedPositions[i*4+0] = glm::vec2(-1.f,  1.f);
		m_normalizedPositions[i*4+1] = glm::vec2( 1.f,  1.f);
		m_normalizedPositions[i*4+2] = glm::vec2( 1.f, -1.f);
		m_normalizedPositions[i*4+3] = glm::vec2(-1.f, -1.f);
#endif
	}

	CreateIdBuffer(gl);

	{
		std::vector<uint16_t> indices(m_spriteCount*6);

		for(uint32_t i = 0; i < m_spriteCount; ++i)
		{
			indices[i*6+0] = i*4+0;
			indices[i*6+1] = i*4+1;
			indices[i*6+2] = i*4+3;

			indices[i*6+3] = i*4+3;
			indices[i*6+4] = i*4+1;
			indices[i*6+5] = i*4+2;
		}

		_gl glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[v_indices]); DEBUG_GL
		_gl glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_DYNAMIC_DRAW); DEBUG_GL
	}

//create null textures
	{
		std::unique_ptr<uint8_t[], void (*)(void*)> ptr((uint8_t*)calloc(m_spriteCount * 4, sizeof(glm::u16vec4)), &std::free);

		_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[v_texCoord]); DEBUG_GL
		_gl glBufferData(GL_ARRAY_BUFFER, m_spriteCount * 4 * sizeof(glm::u16vec4), &ptr[0], GL_DYNAMIC_DRAW); DEBUG_GL

		for(uint32_t i = v_sheetCoordBegin; i < v_sheetCoordEnd; ++i)
		{
			_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[i]); DEBUG_GL
			_gl glBufferData(GL_ARRAY_BUFFER, m_spriteCount * 4 * sizeof(glm::vec2), &ptr[0], GL_DYNAMIC_DRAW); DEBUG_GL
		}
	}

//create positions
	_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[v_positions]); DEBUG_GL
	_gl glBufferData(GL_ARRAY_BUFFER, m_normalizedPositions.size() * sizeof(m_normalizedPositions[0]), &m_normalizedPositions[0], GL_DYNAMIC_DRAW); DEBUG_GL
}

void Material::CreateIdBuffer(GLViewWidget* gl)
{
	std::vector<short> array(m_normalizedPositions.size(), 0);

	for(uint32_t i = 0; i < m_spriteCount; ++i)
	{
		auto pair   = m_spriteVertices[i];

		for(uint32_t k = 0; k < pair.length; ++k)
		{
			array[k+pair.start] = i+1;
		}
	}

	_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[v_spriteId]); DEBUG_GL
	_gl glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(array[0]), &array[0], GL_DYNAMIC_DRAW); DEBUG_GL
}

void Material::Prepare(GLViewWidget* gl)
{
	bool created = false;

	if(!m_vao)
	{
		_gl glGenVertexArrays(1, &m_vao);
		_gl glGenBuffers(VBOc, &m_vbo[0]);

//sprite sheet
		_gl glBindVertexArray(m_vao);
		_gl glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[v_indices]);

		_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[v_positions]);
		_gl glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

		_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[v_spriteId]);
		_gl glVertexAttribIPointer(1, 1, GL_SHORT, 0, nullptr);

		_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[v_texCoord]);
		_gl glVertexAttribPointer(2, 4, GL_UNSIGNED_SHORT, GL_TRUE, 0, nullptr);

		for(int i = 0; i < (int)Tex::Total; ++i)
		{
			_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[v_sheetCoordBegin+i]);
			_gl glVertexAttribPointer(3+i, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		}

		for(int i = 0; i < v_indices; ++i)
			_gl glEnableVertexAttribArray(i);

		GL_ASSERT;

		created = true;
	}

//upload data
	if(m_dirtyTextureFlags || created || m_dirty)
	{
		CreateDefaultArrays(gl);
		m_dirty = false;
	}

	if(m_dirtyTextureFlags || created)
	{
		CreateDefaultArrays(gl);

		auto flags = m_dirtyTextureFlags;
		m_dirtyTextureFlags = 0;

		for(int i = 0; i < (int)Tex::Total; ++i)
		{
			if(!(flags >> i)) continue;

			if(image_slots[i] == nullptr)
				continue;

			auto texCoords = image_slots[i]->m_texCoords->sprites();

			std::vector<glm::vec2> coords(m_normalizedPositions.size());
			memcpy(&coords[0], &m_normalizedPositions[0], sizeof(coords[0]) * coords.size());

			glm::vec4 sheet_size = glm::vec4(image_slots[i]->GetSize(), image_slots[i]->GetSize());

			for(auto & v : coords)
			{
				v.y *= -1;
				v = (v + 1.f) * .5f;
			}

			for(uint32_t j = 0; j < m_spriteCount; ++j)
			{
				glm::vec4 square = glm::vec4(texCoords[j]) / sheet_size;
				auto pair   = m_spriteVertices[j];

				for(uint32_t k = 0; k < pair.length; ++k)
				{
					auto & v = coords[k+pair.start];
					v = glm::mix(glm::vec2(square.x, square.w), glm::vec2(square.z, square.y), v);
				}
			}

			_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[v_sheetCoordBegin+i]);
			_gl glBufferData(GL_ARRAY_BUFFER, coords.size() * sizeof(coords[0]), &coords[0], GL_DYNAMIC_DRAW);
		}
	}

	DEBUG_GL
}

void Material::RenderObjectSheet(GLViewWidget * gl, int frame)
{
	if(isUnlit())
	{
		for(int i = 0; i < (int)Tex::Total; ++i)
		{
			if(image_slots[i])
			{
				RenderSpriteSheet(gl, (Tex)i, frame);
				return;
			}
		}

		return;
	}

	Prepare(gl);
	auto db = GetRenderData(frame);
	RenderSheetBackdrop(gl, db);

	_gl glBindVertexArray(m_vao);

	//draw sprites
	gltfMetallicRoughness::Shader.bind(gl, this);
	m_spriteSheet->BindBoundingBoxes(gl, GL_TEXTURE10);

	gltfMetallicRoughness::Shader.bindMatrix(gl, db.matrix);

	gltfMetallicRoughness::Shader.bindLayer(gl, 4);
	gltfMetallicRoughness::Shader.bindCenter(gl, db.center);

	for(int i = 0; i < (int)Tex::Total; ++i)
	{
		_gl glActiveTexture(GL_TEXTURE0 + i);

		if(image_slots[i] && image_slots[i]->LoadFromFile())
		{
			_gl glBindTexture(GL_TEXTURE_2D, image_slots[i]->GetTexture());
		}
		else
		{
			_gl glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	_gl glDisable(GL_DEPTH_TEST);
	_gl glDrawElements(GL_TRIANGLES, db.elements, GL_UNSIGNED_SHORT, db.offset());

	GL_ASSERT;
}

glm::vec2 Material::SpriteSize(int it) const
{
	for(int i = 0; i < (int)Material::Tex::Total; ++i)
	{
		if(image_slots[i] != nullptr)
		{
			if(image_slots[i]->m_texCoords->sprites().size() < (uint32_t)it)
			{
				auto spr = image_slots[i]->m_texCoords->sprites()[it];
				return glm::vec2(spr.z - spr.x, spr.w - spr.y);
			}

			return {0,0};
		}
	}

	return {0,0};
}

void Material::RenderSpriteSheet(GLViewWidget * gl, Material::Tex image_slot, int frame)
{
	if(image_slots[(int)image_slot] == nullptr || image_slots[(int)image_slot]->LoadFromFile() == false) return;

	Prepare(gl);
	auto db = GetRenderData(frame);
	RenderSheetBackdrop(gl, db);

	_gl glBindVertexArray(m_vao);

	UnlitShader::Shader.bind(gl, this);
	m_spriteSheet->BindBoundingBoxes(gl, GL_TEXTURE10);

	UnlitShader::Shader.bindMatrix(gl, db.matrix);

	UnlitShader::Shader.bindLayer(gl, 8);
	UnlitShader::Shader.bindCenter(gl, db.center);

	UnlitShader::Shader.bindColor(gl, glm::vec4(1, 1, 1, 1)); DEBUG_GL
	UnlitShader::Shader.bindTexCoords(gl, TexCoord(image_slot)); DEBUG_GL

	_gl glActiveTexture(GL_TEXTURE0); DEBUG_GL
	_gl glBindTexture(GL_TEXTURE_2D, image_slots[(int)image_slot]->GetTexture()); DEBUG_GL

	_gl glDisable(GL_DEPTH_TEST); DEBUG_GL
	_gl glDrawElements(GL_TRIANGLES, db.elements, GL_UNSIGNED_SHORT, db.offset()); DEBUG_GL

	DEBUG_GL
}

RenderData Material::GetRenderData(int frame)
{
	if(m_texCoords == nullptr) return {};

	RenderData r;

	if(frame >= 0) frame %= m_spriteCount;

	r.frame       = frame;
	r.elements    = 0;
	r.first       = 0;
	r.center      = (frame >= 0);

	r.matrix      = glm::mat4(1);
//	if(!r.center) r.matrix = glm::translate(r.matrix, -glm::vec3(m_sheetSize.x/2.f, m_sheetSize.y/2.f, 0));

	if(!m_spriteIndices.empty())
	{
		r.elements = frame >= 0? m_spriteIndices[frame].length : m_spriteIndices.back().end();
		r.first    = frame >= 0? m_spriteIndices[frame].start  : 0;
	}

	return r;
}

bool Material::PrepareSheetBackdrop()
{
	if(m_texCoords == nullptr)
		return false;

	if(m_spriteSheet == nullptr)
		m_spriteSheet.reset(new SpriteSheet());

	return true;
}

void Material::RenderSheetBackdrop(GLViewWidget * gl, RenderData const& db)
{
	if(PrepareSheetBackdrop() == false)
		return;

	m_spriteSheet->Prepare(gl, m_texCoords->sprites(), m_sheetSize);
	m_spriteSheet->RenderSheet(gl, db);
}


inline void to_json(nlohmann::json & json, MaterialExtensions const & material)
{
	fx::gltf::detail::WriteField("KHR_materials_pbrSpecularGlossiness", json, material.pbrSpecularGlossiness);
	fx::gltf::detail::WriteField("KHR_materials_unlit", json, material.unlit);

#if KHR_SHEEN
	fx::gltf::detail::WriteField("KHR_materials_sheen", json, material.sheen);
#endif
	fx::gltf::detail::WriteField("KHR_materials_clearcoat", json, material.clearcoat);
}

void from_json(nlohmann::json const& json, MaterialExtensions & material)
{
	fx::gltf::detail::ReadOptionalField("KHR_materials_pbrSpecularGlossiness", json, material.pbrSpecularGlossiness);
	fx::gltf::detail::ReadOptionalField("KHR_materials_unlit", json, material.unlit);

#if KHR_SHEEN
	fx::gltf::detail::ReadOptionalField("KHR_materials_sheen", json, material.sheen);
#endif
	fx::gltf::detail::ReadOptionalField("KHR_materials_clearcoat", json, material.clearcoat);
}

//image is the orignal image not the compressed image.
void PackTexture(fx::gltf::Material::Texture * dst, Image * This, int texCoords, Sprites::Document & doc, PackMemo & memo)
{
	if(This == nullptr)
	{
		dst->index = -1;
		dst->texCoord = -1;
		return;
	}

	auto itr = memo.mapping.find(This);
	if(itr != memo.mapping.end())
	{
		dst->index = itr->second;
		dst->texCoord = texCoords;
		return;
	}

//push sampler
	fx::gltf::Texture texture;
	texture.source = doc.images.size();
	texture.texCoords = This->m_texCoords->Pack(doc, memo);

	doc.textures.push_back(texture);

	uint32_t file_size{};
	auto buffer = This->LoadFileAsArray(file_size);

	fx::gltf::Image   image;

	image.name = This->getFilename();
	image.bufferView = memo.PackBufferView(buffer.release(), file_size, true);

	auto r = doc.images.size();
	doc.images.push_back(image);

	dst->index	  = r;
	dst->texCoord = texCoords;
}

int Material::PackDocument(Material * This, Sprites::Document & doc, PackMemo & memo, glm::ivec4 & frame)
{
	if(This == nullptr)
		return -1;

	auto itr = memo.mapping.find(This);
	if(itr != memo.mapping.end())
		return itr->second;

	const int material_id = doc.materials.size();
	memo.mapping.emplace(This, material_id);

	doc.materials.push_back(*This);
	auto & mat = doc.materials.back();

	TexCoord_t tex_coordsOut;
	auto tex_coords = This->GetTextureCoordinates(tex_coordsOut);

//duplicate it so we can change things around...
	MaterialExtensions ext = This->ext;

#define PackImage(path, index) PackTexture(&path, This->image_slots[(int)index].get(), tex_coords[(int)index], doc, memo)

	PackImage(mat.pbrMetallicRoughness.baseColorTexture, Tex::BaseColor);
	PackImage(ext.pbrSpecularGlossiness.diffuseTexture, Tex::Diffuse);
	PackImage(mat.pbrMetallicRoughness.metallicRoughnessTexture, Tex::MetallicRoughness);
	PackImage(ext.pbrSpecularGlossiness.specularGlossinessTexture, Tex::SpecularGlossiness);
	PackImage(mat.normalTexture, Tex::Normal);
	PackImage(mat.occlusionTexture, Tex::Occlusion);
	PackImage(mat.emissiveTexture, Tex::Emission);

	fx::gltf::detail::WriteField("extensions", mat.extensionsAndExtras, ext);

	frame.x = memo.PackAccessor(This->m_texCoords->sprites());
	frame.y = memo.PackAccessor(This->m_texCoords->cropped());
	frame.z = memo.PackAccessor(tex_coordsOut[0], true);
	frame.w = memo.PackAccessor(tex_coordsOut[1].empty()? tex_coordsOut[0] : tex_coordsOut[1], true);

	return material_id;
}


void Material::LoadExtensionsAndExtras()
{
//copy material
	const nlohmann::json::const_iterator extensions =  extensionsAndExtras.find("extensions");
	if(extensions != extensionsAndExtras.end())
	{
		auto itr = extensions->find("KHR_materials_pbrSpecularGlossiness");

		if(itr != extensions->end())
			KHR::materials::from_json(itr->get<nlohmann::json>(), ext.pbrSpecularGlossiness);

		itr = extensions->find("KHR_materials_unlit");

		if(itr != extensions->end())
			KHR::materials::from_json(itr->get<nlohmann::json>(), ext.unlit);

#if KHR_SHEEN
		itr = extensions->find("KHR_materials_sheen");

		if(itr != extensions->end())
			KHR::materials::from_json(itr->get<nlohmann::json>(), ext.sheen);
#endif

		itr = extensions->find("KHR_materials_clearcoat");

		if(itr != extensions->end())
			KHR::materials::from_json(itr->get<nlohmann::json>(), ext.clearcoat);
	}

	extensionsAndExtras.clear();
}

Material::TexIndex_t Material::GetTextureCoordinates(counted_ptr<Image> image)
{
	TexCoord_t c;

	if(image)
		c[0] = image->m_texCoords->normalizedCrop();

	return GetTextureCoordinates(c);
}

Material::TexIndex_t Material::GetTextureCoordinates(TexCoord_t & texCoords)
{
	Material::TexIndex_t r;

	for(auto i = 0u; i < image_slots.size(); ++i)
	{
		r[i] = -1;

		if(image_slots[i].empty()) continue;

		auto coords = image_slots[i]->m_texCoords->normalizedCrop();

		for(auto j = 0u; j < texCoords.size(); ++j)
		{
			if(texCoords[i] == coords)
			{
				r[i] = j;
				break;
			}
		}

		if(r[i] == -1)
		{
			for(auto j = 0u; j < texCoords.size(); ++j)
			{
				if(texCoords[i].empty())
				{
					texCoords[i] = coords;
					r[i]		 = j;
					break;
				}
			}
		}

		if((uint32_t)r[i] >= texCoords.size())
		{
			throw std::logic_error("incompatible texCoords in image");
		}
	}


	return r;
}
