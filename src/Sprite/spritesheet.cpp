#include "spritesheet.h"
#include "material.h"
#include "Shaders/transparencyshader.h"
#include "Shaders/colorshader.h"
#include "Shaders/defaultvaos.h"
#include "Support/vectoroperations.hpp"
#include "src/widgets/glviewwidget.h"
#include <iostream>
#include <cassert>

#define UNUSED(x) (void)x;

SpriteSheet::~SpriteSheet()
{
	assert(m_vao[0] == 0);
}

void SpriteSheet::Clear(GLViewWidget* gl)
{
	if(m_vao[0] == 0)
		return;

	TransparencyShader::Shader.Release(gl);
	ColorShader::Shader.Release(gl);

	_gl glDeleteVertexArrays(VAOc, m_vao);
	_gl glDeleteBuffers(VBOc, m_vbo);
	_gl glDeleteTextures(TEXc, m_texture);

	memset(m_vbo,     0, sizeof(m_vbo));
	memset(m_texture, 0, sizeof(m_texture));
	memset(m_vao,     0, sizeof(m_vao));

	m_sprites = 0;
	m_size    = glm::i16vec2(0, 0);
	m_length  = 0;
}

struct vertex
{
	glm::i16vec2 position;
	glm::i8vec2  texCoord0;
	uint32_t     sprite_id;
};

inline void PushSprite(std::vector<vertex> & vec, glm::i16vec4 item, uint32_t id)
{
	glm::ivec2 center = SpriteSheet::GetCenter(item);

	vec.push_back({{item.x  - center.x, -(item.y -  center.y)}, {0, 0}, id});
	vec.push_back({{item.z  - center.x, -(item.y -  center.y)}, {1, 0}, id});
	vec.push_back({{item.z  - center.x, -(item.w -  center.y)}, {1, 1}, id});
	vec.push_back({{item.x  - center.x, -(item.w -  center.y)}, {0, 1}, id});
}


void SpriteSheet::Prepare(GLViewWidget* gl, ConstSizedArray<glm::i16vec4> const& sprites, glm::i16vec2 sheet_size)
{
	GL_ASSERT;

	if(m_sprites == sprites.data()
	&& m_size    == sheet_size)
		return;

	m_sprites = sprites.data();
	m_size    = sheet_size;
	m_length  = sprites.size();

	if(m_sprites == nullptr)
		return Clear(gl);

	if(!m_vao[0])
	{
		TransparencyShader::Shader.AddRef();
		ColorShader::Shader.AddRef();

		_gl glGenVertexArrays(VAOc, m_vao);	DEBUG_GL;
		_gl glGenBuffers(VBOc, m_vbo);	DEBUG_GL;
		_gl glGenTextures(TEXc,   m_texture);	DEBUG_GL;

		for(int i = 0; i < VAOc; ++i)
		{
			_gl glBindVertexArray(m_vao[i]);	DEBUG_GL;
			_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[a_vertex]);	DEBUG_GL;

			if(i == v_Squares)
				glDefaultVAOs::BindSquareIndexVBO(gl);

			_gl glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, sizeof(vertex), (void*) offsetof(vertex, position));	DEBUG_GL;
			_gl glVertexAttribIPointer(1, 1, GL_INT, sizeof(vertex), (void*) offsetof(vertex, sprite_id));	DEBUG_GL;
			_gl glVertexAttribPointer(2, 2, GL_BYTE, GL_FALSE, sizeof(vertex), (void*) offsetof(vertex, texCoord0));	DEBUG_GL;

			_gl glEnableVertexAttribArray(0);	DEBUG_GL;
			_gl glEnableVertexAttribArray(1);	DEBUG_GL;
			_gl glEnableVertexAttribArray(2);	DEBUG_GL;
		}

		GL_ASSERT;
	}

	_gl glBindVertexArray(m_vao[0]); DEBUG_GL

//sprite VBO ultimately needs to be all centered etc.
	{
		std::vector<vertex> vec;

		vec.reserve((sprites.size() + 1)*4);

		PushSprite(vec, {0, 0, m_size.x, m_size.y}, 0);

		for(uint32_t i = 0; i < sprites.size(); ++i)
			PushSprite(vec, sprites[i], i+1);

		_gl glBindBuffer(GL_ARRAY_BUFFER, m_vbo[a_vertex]); DEBUG_GL
		_gl glBufferData(GL_ARRAY_BUFFER, vec.size() * sizeof(vec[0]), &vec[0], GL_STATIC_DRAW); DEBUG_GL
	}

//make textures
	{
		std::vector<glm::i16vec4> vec;
		vec.resize(sprites.size()+1);

		vec[0] = glm::i16vec4(0, 0, m_size.x, m_size.y);
		memcpy(&vec[1], &sprites[0], sizeof(vec[0]) * (vec.size()-1));

		auto sheet_center = glm::i16vec4(m_size.x, m_size.y, m_size.x, m_size.y) / (short)2;
		for(uint32_t i = 0; i < vec.size(); ++i)
		{
			vec[i]   -= sheet_center;
			vec[i].y *= -1;
			vec[i].w *= -1;
		}


		_gl glBindBuffer(GL_TEXTURE_BUFFER, m_vbo[a_bounds]); DEBUG_GL
		_gl glBufferData(GL_TEXTURE_BUFFER, vec.size() * sizeof(vec[0]), &vec[0], GL_STATIC_DRAW); DEBUG_GL
	}

	{
		glm::i16vec2 sheet_center(m_size.x/2, m_size.y/2);

		std::vector<glm::i16vec2> vec;
		vec.reserve(sprites.size()+1);

		vec.push_back({0, 0});

		for(uint32_t i = 0; i < sprites.size(); ++i)
		{
			vec.push_back({(sprites[i].x + sprites[i].z) / 2 - sheet_center.x, -((sprites[i].y + sprites[i].w) / 2 - sheet_center.y)});
		//	vec.push_back({1, 1});
		}

		_gl glBindBuffer(GL_TEXTURE_BUFFER, m_vbo[a_centers]); DEBUG_GL
		_gl glBufferData(GL_TEXTURE_BUFFER, vec.size() * sizeof(vec[0]), &vec[0], GL_STATIC_DRAW); DEBUG_GL
	}

	_gl glBindTexture(GL_TEXTURE_BUFFER, m_texture[t_centers]);	DEBUG_GL;
	_gl glTexBuffer(GL_TEXTURE_BUFFER, GL_RG16I, m_vbo[a_centers]);	DEBUG_GL;

	_gl glBindTexture(GL_TEXTURE_BUFFER, m_texture[t_bounds]); DEBUG_GL
	_gl glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA16I, m_vbo[a_bounds]); DEBUG_GL

	_gl glBindTexture(GL_TEXTURE_BUFFER, 0); DEBUG_GL

	GL_ASSERT;
}

void SpriteSheet::BindCenters      (GLViewWidget* gl, uint32_t active_texture)
{
	_gl glActiveTexture(active_texture);
	_gl glBindTexture(GL_TEXTURE_BUFFER, m_texture[t_centers]);
}

void SpriteSheet::BindBoundingBoxes(GLViewWidget* gl, uint32_t active_texture)
{
	_gl glActiveTexture(active_texture);
	_gl glBindTexture(GL_TEXTURE_BUFFER, m_texture[t_bounds]);
}

void SpriteSheet::RenderSheet(GLViewWidget * gl, RenderData db)
{
	GL_ASSERT;

	if(m_sprites == nullptr)
		return;

	const int    elements = db.frame < 0? 6 * length() : 6;
	const int    first    = 6 * (1 + db.frame * (db.frame > 0));
	const void * offset   = (void*) (first * sizeof(short));

//draw backdrop
	_gl glBindVertexArray(m_vao[v_Squares]);

	if(db.frame < 0)
	{
		TransparencyShader::Shader.bind(gl, nullptr);
		if(!db.center) BindCenters(gl, GL_TEXTURE10);
		TransparencyShader::Shader.bindMatrix(gl, db.matrix);

		_gl glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);GL_ASSERT;
	}

//draw sprites boxes
	ColorShader::Shader.bind(gl, nullptr);
	if(!db.center) BindCenters(gl, GL_TEXTURE10);
	ColorShader::Shader.bindMatrix(gl, db.matrix);

	ColorShader::Shader.bindLayer(gl, 3);
	ColorShader::Shader.bindColor(gl, glm::vec4(0, 0, 0, 0)); DEBUG_GL

	_gl glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_SHORT, offset); DEBUG_GL

//draw sprite outlines
	if(db.frame < 0)
	{
		ColorShader::Shader.bindLayer(gl, 2);
		ColorShader::Shader.bindColor(gl, glm::vec4(0, 0, 0, 1));
		_gl glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);GL_ASSERT;
	}

	GL_ASSERT;
}

