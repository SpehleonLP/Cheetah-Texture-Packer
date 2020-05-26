#include "object.h"
#include "document.h"
#include "Support/imagesupport.h"
#include "Shaders/defaultvaos.h"
#include "Shaders/transparencyshader.h"
#include "Shaders/blitshader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fx/gltf.h>
#include <cctype>
#include <cstring>
#include <climits>

#include "widgets/glviewwidget.h"

static bool CheckDynamics(std::string & error, float & size_ratio, CountedSizedArray<glm::i16vec4> A, CountedSizedArray<glm::i16vec4> B);

std::string Object::IsImageCompatible(counted_ptr<Image> image, Material::Tex slot)
{
	if(slot == Material::Tex::None)
		return "No such image slot";

	image->LoadFromFile();

	for(int i = 0; i < (int)Material::Tex::Total; ++i)
	{
		auto img = material.image_slots[i].get();

		if(img == nullptr)
			continue;

		if(i == (int)slot)
		{
			if(img == image.get())
				return "Given image already selected for this slot.";

			continue;
		}

		if(img->m_sprites.size() != image->m_sprites.size())
			return "number of images must be consistent on all sprite sheets used by object.";

//try to merge shit...
		if( image->m_sprites.merge(img->m_sprites)
		&& !image->m_cropped.merge(img->m_cropped))
			image->m_cropped.merge(image->m_sprites);

		image->m_normalized.merge(img->m_normalized);
		if(!image->m_normalizedPositions.merge(img->m_normalizedPositions))
			return "texture coordinates of all textures do not properly align";
	}

	return std::string();
}

void Object::UpdateCachedArrays()
{
	int priority[] =
	{
		(int)Material::Tex::BaseColor,
		(int)Material::Tex::Diffuse,
		(int)Material::Tex::Normal,
		(int)Material::Tex::MetallicRoughness,
		(int)Material::Tex::SpecularGlossiness,
		(int)Material::Tex::Occlusion,
		(int)Material::Tex::Emission,
		(int)Material::Tex::None
	};

	CountedSizedArray<glm::i16vec4> positions;
	CountedSizedArray<glm::i16vec4> crop;
	CountedSizedArray<glm::u16vec4> normalized;
	CountedSizedArray<glm::u16vec4> normPos;

	for(int const* p = priority; *p != -1; ++p)
	{
		if(material.image_slots[*p] == nullptr)
			continue;

		if(positions.empty())
		{
			positions  = material.image_slots[*p]->m_sprites;
			crop       = material.image_slots[*p]->m_cropped;
			normalized = material.image_slots[*p]->m_normalized;
			normPos    = material.image_slots[*p]->m_normalizedPositions;
		}

		if(material.image_slots[*p]->m_normalizedPositions == normPos)
			material.TexCoord((Material::Tex)*p) = 0;
		else
		{
			throw std::logic_error("Sprites do not properly align.");
		}
	}




}
#if 0
void Object::UpdateImages(Document* doc)
{
	int priority[] =
	{
		(int)Material::Tex::BaseColor,
		(int)Material::Tex::Diffuse,
		(int)Material::Tex::Normal,
		(int)Material::Tex::MetallicRoughness,
		(int)Material::Tex::SpecularGlossiness,
		(int)Material::Tex::Occlusion,
		(int)Material::Tex::Emission,
		(int)Material::Tex::None
	};

	CountedSizedArray<glm::i16vec4> positions;
	CountedSizedArray<glm::i16vec4> crop;
	CountedSizedArray<glm::u16vec4> normalized;
	CountedSizedArray<glm::u16vec4> normPos;

	for(int const* p = priority; *p != -1; ++p)
	{
		if(material.image_slots[*p] == nullptr)
			continue;

		if(positions.empty())
		{
			positions  = material.image_slots[*p]->m_sprites;
			crop       = material.image_slots[*p]->m_cropped;
			normalized = material.image_slots[*p]->m_normalized;
			normPos    = material.image_slots[*p]->m_normalizedPositions;
		}

		if(material.image_slots[*p]->m_normalizedPositions == normPos)
			material.TexCoord((Material::Tex)*p) = 0;
		else
		{
			throw std::logic_error("Sprites do not properly align.");
		}
	}

	struct vertex
	{
		glm::i16vec2 position;
		glm::i16vec2 crop;
		glm::i16vec2 center;
		glm::u16vec2 texCoord0{0, 0};
		glm::u16vec2 texCoord1{0, 0};
		glm::u16vec2 spriteCoords;
	};

	std::vector<vertex> buffer;
	buffer.reserve(positions.size()*4);

	for(uint32_t i = 0; i < positions.size(); ++i)
	{
		vertex v[4];

		v[0].position = glm::i16vec2(positions[i].x, positions[i].y);
		v[1].position = glm::i16vec2(positions[i].z, positions[i].y);
		v[2].position = glm::i16vec2(positions[i].z, positions[i].w);
		v[3].position = glm::i16vec2(positions[i].x, positions[i].w);

		v[0].crop = glm::i16vec2(crop[i].x, crop[i].y);
		v[1].crop = glm::i16vec2(crop[i].z, crop[i].y);
		v[2].crop = glm::i16vec2(crop[i].z, crop[i].w);
		v[3].crop = glm::i16vec2(crop[i].x, crop[i].w);

		v[0].center = glm::ivec2(positions[i].x + positions[i].z, positions[i].y + positions[i].w) / 2;
		v[1].center = v[0].center;
		v[2].center = v[0].center;
		v[3].center = v[0].center;

		v[0].spriteCoords = glm::u16vec2(normalized[i].x, normalized[i].y);
		v[1].spriteCoords = glm::u16vec2(normalized[i].z, normalized[i].y);
		v[2].spriteCoords = glm::u16vec2(normalized[i].z, normalized[i].w);
		v[3].spriteCoords = glm::u16vec2(normalized[i].x, normalized[i].w);

		buffer.push_back(v[0]);
		buffer.push_back(v[1]);
		buffer.push_back(v[2]);
		buffer.push_back(v[3]);
	}

	auto gl = doc->GetViewWidget();

	gl->makeCurrent();

	if(!m_vao[0])
	{
		_gl glGenVertexArrays(3, m_vao);
		_gl glGenBuffers(VBOc, &m_vbo[0]);

//prep array....
		_gl glBindVertexArray(m_vao[0]);
		glDefaultVAOs::BindSquareIndexVBO(gl);

		_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[renderVBO]);

		_gl glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
		_gl glVertexAttribPointer(1, 2, GL_SHORT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, center));
		_gl glVertexAttribPointer(2, 2, GL_SHORT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, crop));
		_gl glVertexAttribPointer(3, 2, GL_SHORT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, texCoord0));
		_gl glVertexAttribPointer(4, 2, GL_SHORT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, texCoord1));
		_gl glVertexAttribPointer(5, 2, GL_SHORT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, spriteCoords));

		_gl glEnableVertexAttribArray(0);
		_gl glEnableVertexAttribArray(1);
		_gl glEnableVertexAttribArray(2);
		_gl glEnableVertexAttribArray(3);
		_gl glEnableVertexAttribArray(4);
	}

	_gl glBindVertexArray(m_vao[0]);
	_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[renderVBO]);

	_gl glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(buffer[0]), &buffer[0], GL_DYNAMIC_DRAW);
}

void Object::Render(GLViewWidget * gl, Material::Tex texture, int frame, int outline)
{
	if(m_sprites.empty())
		return;

	GL_ASSERT;

	if(frame >= 0) frame %= m_sprites.size();

	uint32_t elements = frame < 0? 6 * m_sprites.size() : 6;
	intptr_t first = 6 * (1 + frame * (frame > 0));
	void * offset   = (void*)(2 * first);
	bool center = (frame >= 0);

	glm::mat4 matrix = glm::mat4(1);

	if(!center) matrix =  glm::translate(matrix, -glm::vec3(m_size.x/2.f, m_size.y/2.f, 0));

	if(outline >= 0)
	{
		BlitShader::Shader.bind(gl, nullptr);
		BlitShader::Shader.bindLayer(gl, 1);
		BlitShader::Shader.bindCenter(gl, frame >= 0);
		BlitShader::Shader.bindColor(gl, glm::vec4(1, 1, 0, 0));GL_ASSERT;

		_gl glBindVertexArray(m_vao[outline % 2]);
		_gl glDrawArrays(GL_TRIANGLES, first, elements);
	}
	else
	{
//draw backdrop
		_gl glBindVertexArray(m_vao[0]);

		if(frame < 0)
		{
			TransparencyShader::Shader.bind(gl);
			TransparencyShader::Shader.bindCenter(gl, center);
			TransparencyShader::Shader.bindMatrix(gl, matrix);

			_gl glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);GL_ASSERT;
		}

//draw sprites boxes
		BlitShader::Shader.bind(gl, nullptr);
		BlitShader::Shader.bindCenter(gl, center);
		BlitShader::Shader.bindMatrix(gl, matrix);

		BlitShader::Shader.bindLayer(gl, 3);
		BlitShader::Shader.bindColor(gl, glm::vec4(0, 0, 0, 0));GL_ASSERT;

		_gl glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_SHORT, offset);GL_ASSERT;

//draw sprite outlines
		_gl glBindVertexArray(m_vao[1]);

		if(frame < 0)
		{
			BlitShader::Shader.bindLayer(gl, 2);
			BlitShader::Shader.bindColor(gl, glm::vec4(1, 1, 1, 1));
			_gl glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);GL_ASSERT;
		}

//draw sprites
		_gl glDisable(GL_DEPTH_TEST);
		BlitShader::Shader.bindLayer(gl, 4);
		BlitShader::Shader.bindTexture(gl, m_texture);GL_ASSERT;
		BlitShader::Shader.clearColor(gl);
		_gl glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_SHORT, offset);
	}
	GL_ASSERT;
}
#endif