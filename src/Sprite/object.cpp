#include "object.h"
#include "document.h"
#include "Support/imagesupport.h"
#include "Support/packaccessor.h"
#include "Support/unpackmemo.h"
#include "Sprite/spritejson.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fx/gltf.h>
#include <cctype>
#include <cstring>
#include <climits>

#include "widgets/glviewwidget.h"

void Object::RenderAttachments(GLViewWidget *, int )
{


}

Object::Object(ImageManager * gl, Sprites::Sprite const& spr, Sprites::Document const& doc, UnpackMemo & memo) :
	gl(gl->GetGL())
{
	name = counted_string::MakeShared(spr.name);

//copy animations
	animations.resize(spr.animations.size());

	for(uint32_t i = 0; i < animations.size(); ++i)
	{
		auto r = UncountedWrap(new Animation());

		r->name       =  counted_string::MakeShared(spr.animations[i].name);
		r->fps        = spr.animations[i].fps;
		r->loop_start = spr.animations[i].loop_start;
		r->loop_end   = spr.animations[i].loop_end;

		if(!spr.animations[i].frames.empty())
			r->frames = shared_array<uint16_t>::FromArray(
				&spr.animations[i].frames[0],
				 spr.animations[i].frames.size());

		animations[i] = std::move(r);
	}

//copy attachments
	attachments.resize(spr.attachments.size());

	auto att_data = memo.GetAccessor_i16vec2(doc, spr.frames.attachments);

	for(uint32_t point = 0; point < attachments.size(); ++point)
	{
		attachments[point].name = counted_string::MakeShared(spr.attachments[point]);
		attachments[point].dirty = true;

		attachments[point].coords.resize(spr.frames.count);

		for(uint32_t frame = 0; frame < attachments[frame].coords.size(); ++frame)
			attachments[point].coords[frame] = att_data[frame * attachments.size() + point];
	}

	material.reset(new Material(gl, spr, doc, memo));
}

Object::Object(GLViewWidget * gl) : gl(gl) {}
Object::~Object() { material->Clear(gl); }


int Object::PackDocument(Sprites::Document & doc, PackMemo & memo)
{
	material->Prepare(gl);

	Sprites::Sprite sprite;

	glm::ivec4 frame{-1};

	sprite.name = name.toStdString();
	sprite.material = Material::PackDocument(material.get(), doc, memo, frame);

	sprite.frames.AABB = frame.x;
	sprite.frames.crop = frame.y;
	sprite.frames.texCoord0 = frame.z;
	sprite.frames.texCoord1 = frame.w;
	sprite.frames.count = noFrames();

//pack attachments
	sprite.attachments.resize(attachments.size());
	for(uint32_t i = 0; i < attachments.size(); ++i)
	{
		sprite.attachments[i] = attachments[i].name.toStdString();
	}

//pack attachment locations
	std::vector<glm::i16vec2> att_data(noFrames() * attachments.size());

	for(auto frame = 0u; frame < noFrames(); ++frame)
	{
		glm::i16vec2 * array = att_data.data() + frame * attachments.size();

		for(auto point = 0u; point < attachments.size(); ++point)
		{
			array[point] = 	attachments[point].coords[frame];
		}
	}

	sprite.frames.attachments = memo.PackAccessor(att_data);

//push animations
	sprite.animations.resize(animations.size());

	for(uint32_t i = 0; i < sprite.animations.size(); ++i)
	{
		sprite.animations[i] = animations[i]->PackDocument();
	}

	doc.sprites.push_back(sprite);
	return doc.sprites.size()-1;
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

	shared_array<glm::i16vec4> positions;
	shared_array<glm::i16vec4> crop;
	shared_array<glm::u16vec4> normalized;
	shared_array<glm::u16vec4> normPos;

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
		BlitShader::Shader.bindColor(gl, glm::vec4(1, 1, 0, 0));

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

			_gl glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		}

//draw sprites boxes
		BlitShader::Shader.bind(gl, nullptr);
		BlitShader::Shader.bindCenter(gl, center);
		BlitShader::Shader.bindMatrix(gl, matrix);

		BlitShader::Shader.bindLayer(gl, 3);
		BlitShader::Shader.bindColor(gl, glm::vec4(0, 0, 0, 0));

		_gl glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_SHORT, offset);

//draw sprite outlines
		_gl glBindVertexArray(m_vao[1]);

		if(frame < 0)
		{
			BlitShader::Shader.bindLayer(gl, 2);
			BlitShader::Shader.bindColor(gl, glm::vec4(1, 1, 1, 1));
			_gl glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		}

//draw sprites
		_gl glDisable(GL_DEPTH_TEST);
		BlitShader::Shader.bindLayer(gl, 4);
		BlitShader::Shader.bindTexture(gl, m_texture);
		BlitShader::Shader.clearColor(gl);
		_gl glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_SHORT, offset);
	}
	
}
#endif
