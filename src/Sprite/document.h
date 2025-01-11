#ifndef DOCUMENT_H
#define DOCUMENT_H
#include <fx/gltf.h>
#include <fx/extensions/khr_materials.h>
#include "commandinterface.hpp"
#include "packersettings.h"
#include "Support/shared_array.hpp"
#include "Support/counted_string.h"
#include "Support/counted_ptr.hpp"
#include "image.h"
#include "imagemanager.h"
#include "object.h"
#include <QFileInfo>
#include <chrono>
#include <algorithm>
#include <memory>
#include <atomic>

namespace Sprites
{
struct Document;
}

class CommandInterface;
class MainWindow;
class GLViewWidget;
class Image;
struct Document;

struct Document
{
	static std::unique_ptr<Document> OpenFile(GLViewWidget*gl, QFileInfo const& path);
	static std::unique_ptr<Document> OpenFile(GLViewWidget*gl, QString const& path) { return OpenFile(gl, QFileInfo(path)); }

	Document(GLViewWidget * gl, Sprites::Document const&, const std::string & documentFilePath);
	Document(GLViewWidget*gl) : imageManager(ImageManager::Factory(gl)) {}

	Sprites::Document ToExportDocument();

	counted_ptr<ImageManager> imageManager;
	PackerSettings			  settings;

//object -> materials
	std::vector<counted_ptr<Object>>   objects;
	MainWindow *          m_window{};

	QString Name() const { return m_title; };

	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) delete this; }

	template<typename T, typename...Args>
	CommandInterface * addCommand(Args&&... args)
	{
		std::unique_ptr<T> r;
		try
		{
			r = std::make_unique<T>(this, std::move(args)...);
			return addCommand(std::move(r));
		}
		catch(std::exception& e)
		{
			OnError(e.what());
			return nullptr;
		}
	}

	CommandInterface * addCommand(std::unique_ptr<CommandInterface> );

	void editUndo();
	void editRedo();

	bool CanUndo() const { return commandId > 0; }
	bool CanRedo() const { return commandId < commandList.size(); }

	void RenderAnimation(GLViewWidget *,  Object * obj, int frame = -1);
	void RenderPackedTextures(GLViewWidget *, int) {}

	void ClearAnimation() { animation = nullptr; }
	void OnError(std::string const& what);

	GLViewWidget * GetViewWidget() const;

	bool SaveFile(QFileInfo const& path);
	bool SaveFile(QString const& path) { return SaveFile(QFileInfo(path)); }

	QFileInfo m_path;
	QString    m_title{"<untitled>"};

private:
	Sprites::Document PackDocument();

typedef std::chrono::steady_clock::time_point time_point;
	mutable std::atomic<int> m_refCount{1};

	std::vector<std::unique_ptr<CommandInterface> > commandList;
	uint32_t                                        commandId{0};

	void     * animation{};
	time_point animation_start{};

//image management
};

#endif // DOCUMENT_H
