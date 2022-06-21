#ifndef SPRITEOBJECT_H
#define SPRITEOBJECT_H
#include "Support/countedsizedarray.hpp"
#include "Support/counted_string.h"
#include "material.h"
#include "animation.h"

struct Document;

namespace Sprites
{
struct Document;
struct Sprite;
}

struct PackMemo;
struct UnpackMemo;

struct Object
{
	struct Attachment
	{
		counted_string            name;
		std::vector<glm::i16vec2> coords;
		bool dirty{};
	};

	Object(ImageManager * gl, Sprites::Sprite const&, Sprites::Document const& doc, UnpackMemo & memo);
	Object(GLViewWidget * gl);
	~Object();

	counted_string                      name;
	std::unique_ptr<Material>           material{new Material()};
	std::vector<counted_ptr<Animation>> animations;
	std::vector<Attachment>             attachments;

	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) delete this; }

	void RenderAttachments(GLViewWidget *, int attachment = -1);
	void SetAttachment(int selected_attachment, glm::ivec2 position);

	inline std::string IsImageCompatible(counted_ptr<Image> const& image, Material::Tex slot) { return material->IsImageCompatible(slot, image); }
	inline void SetImage(counted_ptr<Image> const& image, counted_ptr<Image> * slot) { material->SetImage(image, slot); }

//loosen coupling
	inline auto RenderObjectSheet(GLViewWidget *gl, int frame = -1) { return material->RenderObjectSheet(gl, frame); }
	inline auto RenderSpriteSheet(GLViewWidget * gl, Material::Tex image_slot, int frame = -1) { return material->RenderSpriteSheet(gl, image_slot, frame);  }

	int PackDocument(Sprites::Document & doc, PackMemo & mapping);
	inline uint32_t noFrames() const  { return material->noFrames(); }

private:
	mutable std::atomic<int> m_refCount{1};
	GLViewWidget * gl;

	bool attachments_dirty{};

	uint32_t m_attachmentVAO{};
	uint32_t m_attachmentVBO{};
};



#endif // SPRITEOBJECT_H
