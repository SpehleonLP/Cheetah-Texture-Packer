#include "document.h"
#include "mainwindow.h"
#include "Sprite/object.h"
#include "ui_mainwindow.h"
#include "Sprite/spritejson.h"
#include "widgets/glviewwidget.h"
#include "Shaders/transparencyshader.h"
#include "Shaders/unlitshader.h"
#include "Shaders/gltfmetallicroughness.h"
#include "Shaders/defaultvaos.h"
#include "Support/packaccessor.h"
#include "Support/unpackmemo.h"
#include <glm/gtc/matrix_transform.hpp>
#include <QMessageBox>


CommandInterface * Document::addCommand(std::unique_ptr<CommandInterface> it)
{
	commandList.resize(commandId);
	commandList.push_back(std::move(it));
	commandId++;

	m_window->OnDocumentChanged();

	return commandList.back().get();
}

void Document::editUndo()
{
	if(commandId > 0)
	{
		--commandId;
		commandList[commandId]->RollBack();

		m_window->OnDocumentChanged();
	}
}

void Document::editRedo()
{
	if(commandId < commandList.size())
	{
		commandList[commandId]->RollForward();
		++commandId;

		m_window->OnDocumentChanged();
	}
}

void Document::OnError(std::string const& what)
{
	m_window->DisplayError(what);
}

Sprites::Document Document::PackDocument()
{
	Sprites::Document r;

	r.asset.copyright = "Lifaundi Official 2020";
	r.asset.generator = "Cheetah";
	r.asset.minVersion = "0.0";

	PackMemo memo;

	for(uint32_t i = 0; i < objects.size(); ++i)
		objects[i]->PackDocument(r, memo);

	memo.PackDocument(r);

	return r;
}

std::unique_ptr<Document> Document::OpenFile(GLViewWidget * gl, QFileInfo const& path)
{
	std::unique_ptr<Document> r;

	try
	{
		r.reset(new Document(gl, Sprites::LoadFromBinary(path.filePath().toStdString(), false)));
	}
	catch(std::exception & e)
	{
		QMessageBox::critical(gl->w, "Error OpeningFile File", e.what());
		return nullptr;
	}

	if(r)
	{
		r->m_path  = path;
		r->m_title = path.fileName();
	}

	return r;
}

bool Document::SaveFile(QFileInfo const& Path)
{
	try
	{
		Sprites::Save(PackDocument(), Path.filePath().toStdString(), true);
	}
	catch(std::exception & e)
	{
		QMessageBox::critical(m_window, "Error Saving File", e.what());
		return false;
	}

	m_path = Path;
	m_title = Path.fileName();

	m_window->SetAsterisk(false);
	return true;
}

/*
void Document::RenderObjectSheet(GLViewWidget * gl, Object* object, int frame)
{
	if(object == nullptr || gl == nullptr)
		return;

	if(frame > 0)
		object->RenderSheetBackdrop(gl, frame);

	if(!object->material.unlit.is_empty)
	{
		if(object->material.image_slots[(int)Material::Tex::BaseColor])
			object->RenderSpriteSheet(gl, frame, (int)Material::Tex::BaseColor);

		return;
	}


}

void Document::RenderSpriteSheet(GLViewWidget * gl, Image * image, int frame)
{
	if(image == nullptr)
		return;

	image->Render(gl, frame, -1);
}

void Document::RenderAttachments(GLViewWidget * gl, Object* object, int )
{
	if(object == nullptr || gl == nullptr)
		return;

	for(int i = 0; i < (int)Material::Tex::Total; ++i)
	{
		if(object->material.image_slots[i] != nullptr)
		{
			RenderSpriteSheet(gl, object->material.image_slots[i].get(), -1);
			break;
		}
	}
}

*/

void Document::RenderAnimation(GLViewWidget * gl,  Object* object, int id)
{
	if(object == nullptr || gl == nullptr || (uint32_t)id >= object->animations.size())
	{
		animation = nullptr;
		return;
	}

	auto anim = object->animations[id].get();

	if(anim != animation)
	{
		animation = anim;
		animation_start = std::chrono::steady_clock::now();
	}

	if(anim->frames.size() == 0)
		return;

	float time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - animation_start).count();
	time *= 1e-6;
	int frame = anim->frames[(int)(time * anim->fps) % anim->frames.size()];

	object->RenderObjectSheet(gl, frame);
}

GLViewWidget * Document::GetViewWidget() const
{
	return m_window->ui->viewWidget;
}

Document::Document(GLViewWidget * gl, Sprites::Document const& doc) :
	imageManager(gl)
{
	UnpackMemo memo;

	objects.reserve(doc.sprites.size());

	for(auto &item : doc.sprites)
		objects.push_back(UncountedWrap(new Object(gl, item, doc, memo)));
}

Sprites::Document ToExportDocument()
{
	Sprites::Document r;



	return r;
}
