#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGlobal>

#include <QMainWindow>
#include <QListWidget>

#include <QDropEvent>
#include <QUrl>
#include <QDrag>
#include <QLabel>
#include "imagepacker.h"

namespace Ui
{
    class MainWindow;
}

class QShortcut;
class SettingsPanel;
class View;
struct Document;
class SpriteModel;


class MainWindow : public QMainWindow
{
friend class SettingsPanel;
        Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	std::unique_ptr<Document>    document;
	std::unique_ptr<SpriteModel> model;

	QShortcut * m_delete{};
	QString outDir;
	QString outFile;
	QString outFormat;
	QString imgFormat;

	Ui::MainWindow *ui{};

	float SetZoom(float);
	float GetZoom() { return m_zoom; };

	glm::vec2 GetScroll();
	void      SetScroll(glm::vec2 scroll);

	void setZoomText(float);
	bool selectTiles(int texture, QRect rect, Qt::KeyboardModifiers	 keys);
	void packerRepaint();

	void OnDocumentChanged();
	void DisplayError(std::string const& what);
	std::string GetImage();

	bool SetAsterisk(bool value);

protected:
//	void dropEvent(QDropEvent *event);
//	void dragEnterEvent(QDragEnterEvent *event);

public:
	bool fileOpen();
	void deleteSelectedTiles();
	void packerUpdate();
	void updateAuto();
	void updateAplhaThreshold();
	void exportImage();
	void clearTiles();

private:
	bool fileSave();
	bool fileSaveAs();

	void ImportSprite();
	std::string GetSpritePath();
	void displayStatus(quint64 image_area, quint64 packer_area, quint64 neededArea, int missingImages, int mergedImages);

	View   * m_view{};
	QLabel * m_status{};
	QLabel * m_scale{};

	SettingsPanel * prefs{};

	void RecurseDirectory(const QString &dir);
	QString topImageDir;
	QString defaultSpritePath;

	bool exporting{};
	bool m_asterisk{};
	bool recursiveLoaderDone{};
	int recursiveLoaderCounter{};
	float m_zoom{1.f};
	void addDir(QString dir);
};

#endif // MAINWINDOW_H
