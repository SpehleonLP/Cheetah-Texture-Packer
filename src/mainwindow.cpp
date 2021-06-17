#include "mainwindow.h"
#include "Sprite/document.h"
#include "ui_mainwindow.h"
#include "settingspanel.h"
#include "packersettings.h"
#include "widgets/spritemodel.h"
#include "commandlist.h"
#include "support.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QMimeData>
#include <QPushButton>
#include <QImageWriter>
#include <QShortcut>
#include <QKeySequence>
#include <QDateTime>
#include <QMenu>
#include <QPainter>
#include <iostream>
#include <cassert>


typedef void (QComboBox::*QComboBoxChanged)(int);
typedef void (QSpinBox::*QSpinBoxChanged)(int);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
	prefs(0L)
{
	m_delete = new QShortcut(QKeySequence(QKeySequence::Delete), this);
	m_status = new QLabel();
	m_scale  = new QLabel();

    exporting = false;
    ui->setupUi(this);
	ui->viewWidget->w = this;

	prefs = new SettingsPanel(*this);

	document.reset(new Document(ui->viewWidget));
	document->m_window = this;

//configure tree
	model.reset(new SpriteModel(this));
	model->window = this;

	ui->treeView->setModel(model.get());
	ui->treeView->setColumnWidth(0, ui->treeView->columnWidth(0)*2);
	ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treeView, &QTreeView::customContextMenuRequested, this, [this](const QPoint & point)
	{
		QModelIndex index = ui->treeView->indexAt(point);
		if(index.isValid())
		{
			model->onCustomContextMenu(index, ui->treeView->viewport()->mapToGlobal(point));
		}
	});

	connect(ui->treeView, &QTreeView::doubleClicked, model.get(), &SpriteModel::doubleClicked);
	connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](auto const& selected, auto const& )
	{
		if(selected.indexes().size())
			model->activated(selected.indexes().first());
	});

//	ui->scrollArea->setWidget(m_view);
//	ui->scrollArea->setAutoFillBackground(false);

	outDir = QDir::homePath();

    exporting = false;
	/*
    ui->widget->scaleBox = ui->d_scale;
    tabifyDockWidget(ui->dockPreferences, ui->dockExport);
    ui->dockPreferences->raise();
	*/

	ui->statusBar->addWidget(m_status);
	ui->statusBar->addPermanentWidget(m_scale);

	connect(ui->editUndo,    &QAction::triggered,  this, [this]() { document->editUndo(); });
	connect(ui->editRedo,    &QAction::triggered,  this, [this]() { document->editRedo(); });
	connect(ui->fileImportSpr,  &QAction::triggered,  this, &MainWindow::ImportSprite);

//	QComboBoxChanged qComboBoxChanged = &QComboBox::currentIndexChanged;
//connnect things state set to ensure auto update doesn't go off.
	connect(ui->c_previewWithImages, &QAction::toggled,               this, &MainWindow::updateAuto);

	//QSpinBoxChanged qSpinBoxChanged = &QSpinBox::valueChanged;




    connect(ui->fileClose,   &QAction::triggered, this, &MainWindow::clearTiles);

    connect(ui->zoomIn , &QAction::triggered, [this]() { SetZoom(m_zoom * 9 / 8.f); } );
    connect(ui->zoomOut, &QAction::triggered, [this]() { SetZoom(m_zoom * 8 / 9.f);  } );

//	connect(ui->tilesList, &QListWidget::itemSelectionChanged, this, &MainWindow::packerRepaint);


	connect(m_delete, &QShortcut::activated, this, &MainWindow::deleteSelectedTiles);

	connect(ui->fileNew, &QAction::triggered, []()
	{
		MainWindow * window = new MainWindow();
		window->show();
	});

	connect(ui->fileOpen, &QAction::triggered, this, &MainWindow::fileOpen);
	connect(ui->fileSave, &QAction::triggered, this, &MainWindow::fileSave);
	connect(ui->fileSaveAs, &QAction::triggered, this, &MainWindow::fileSaveAs);
	connect(ui->m_about, &QAction::triggered, [this]() {
		QMessageBox::information(this, QString("About %1").arg(windowTitle()),
		"Original by github.com/scriptum\nUpdated by github.com/pdjeeves\nUsed code by Hyllian - sergiogdb@gmail.com");
	});
	connect(ui->m_aboutQt, &QAction::triggered, [this]() { QMessageBox::aboutQt(this); });

//set packer settings
	updateAuto();
}

MainWindow::~MainWindow()
{
    delete ui;
	delete prefs;
}

void MainWindow::setZoomText(float zoom)
{
	m_scale->setText(QString("%1%").arg((int) (zoom*100)));
}

void MainWindow::RecurseDirectory(const QString &dir)
{
    QDir dirEnt(dir);
    QFileInfoList list = dirEnt.entryInfoList();
    for(int i = 0; i < list.count() && !recursiveLoaderDone; i++)
    {
        recursiveLoaderCounter++;
        QFileInfo info = list[i];

        QString filePath = info.filePath();
        QString fileExt = info.suffix().toLower();
        QString name = dir + QDir::separator();
        if(info.isDir())
        {
            // recursive
            if(info.fileName() != ".." && info.fileName() != ".")
            {
                RecurseDirectory(filePath);
            }
        }
        else
            if(g_extensions.contains(fileExt))
            {
                if(!QFile::exists(name + info.completeBaseName() + QString(".atlas")))
                {
					/*
                    ui->tilesList->addItem(filePath.replace(topImageDir, ""));
                    packerData *data = new packerData();
                    data->listItem = ui->tilesList->item(ui->tilesList->count() - 1);
					data->file = info.fileName();
                    data->path = info.absoluteFilePath();
                    packer.addItem(data->path, data);*/
                }
            }
        if(recursiveLoaderCounter == 500)
        {
            if(QMessageBox::No ==
                    QMessageBox::question(
                        this,
                        tr("Directory is too big"),
                        tr("It seems that directory <b>") + topImageDir +
                        tr("</b> is too big. "
                           "Loading may take HUGE amount of time and memory. "
                           "Please, check directory again. <br>"
                           "Do you want to continue?"),
                        QMessageBox::Yes,
                        QMessageBox::No))
            {
                recursiveLoaderDone = true;
                recursiveLoaderCounter++;
                continue;
            }
            ui->c_previewWithImages->setChecked(false);
        }
    }
}

bool MainWindow::SetAsterisk(bool value)
{
	m_asterisk = value;
	setWindowTitle(tr("%1%2 - Map Editor").arg(value? "*" : "", document->Name()));
	return true;
}

static QDir g_sprFilePath = QDir::home();

bool MainWindow::fileOpen()
{
	GLViewWidget::TimerState pause_timer(ui->viewWidget);

	QFileDialog dialog(this, tr("Save Sprite File"));
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setNameFilter("Sprite File (*.lf_spr)");
	dialog.setDirectory(g_sprFilePath);

	std::unique_ptr<Document> doc;

	bool accepted;
	while ((accepted = (dialog.exec() == QDialog::Accepted)) && nullptr == (doc = Document::OpenFile(ui->viewWidget, dialog.selectedFiles().first()))) {}

	if(!accepted)
		return false;

	g_sprFilePath = dialog.directory();
	document = std::move(doc);
	document->m_window = this;
	SetAsterisk(false);

	return true;
}

bool MainWindow::fileSave()
{
	if(!document->m_path.isFile())
		return fileSaveAs();

	return document->SaveFile(document->m_path);
}

bool MainWindow::fileSaveAs()
{
	GLViewWidget::TimerState pause_timer(ui->viewWidget);

	QFileDialog dialog(this, tr("Save Sprite File"));
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setNameFilter("Sprite File (*.lf_spr)");
	dialog.setDirectory(g_sprFilePath);

	bool accepted;
	while ((accepted = (dialog.exec() == QDialog::Accepted)) && !document->SaveFile(dialog.selectedFiles().first())) {}

	if(!accepted)
		return false;

	g_sprFilePath = dialog.directory();

	return true;
}


void MainWindow::addDir(QString dir)
{
    //FIXME
    //this is messy hack due to difference between QFileDialog and QFileInfo dir separator in Windows
    if(QDir::separator() == '\\')
    {
        topImageDir = dir.replace("\\", "/") + "/";
    }
    else
    {
        topImageDir = dir + "/";
    }


	outDir = dir;

    recursiveLoaderCounter = 0;
    recursiveLoaderDone = false;
    //packer.clear();
    RecurseDirectory(dir);
    QFileInfo info(dir);
	outFile = info.baseName();
}


void MainWindow::OnDocumentChanged()
{
	model->layoutChanged();
	ui->viewWidget->need_repaint(false);
}

void MainWindow::DisplayError(std::string const& what)
{
	QMessageBox::warning(this, "Cheetah Error", what.c_str());
}

#include <QImageReader>
#include <QStandardPaths>
#include <QMimeDatabase>

#if USE_BASISU

extern const char * g_ReadMimeTypes[];
extern const char * g_WriteMimeTypes[];

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
	QStringList mimeTypeFilters;

	for(const char ** p = acceptMode == QFileDialog::AcceptOpen
		? g_ReadMimeTypes : g_WriteMimeTypes; **p == '\0'; ++p)
		mimeTypeFilters << *p;

    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/bmp");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("bmp");
}

#else

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode, QByteArrayList supportedMimeTypes = {})
{
	if(supportedMimeTypes.empty())
	{
		if(acceptMode == QFileDialog::AcceptOpen)
			supportedMimeTypes = QImageReader::supportedMimeTypes();
		else
			supportedMimeTypes = QImageWriter::supportedMimeTypes();
	}

	QStringList mimeTypeFilters;
	foreach(const QByteArray& mimeTypeName, supportedMimeTypes)
	{
		mimeTypeFilters.append(mimeTypeName);
	}

	mimeTypeFilters.sort(Qt::CaseInsensitive);

	// compose filter for all supported types
	QMimeDatabase mimeDB;
	QStringList allSupportedFormats;
	for(const QString& mimeTypeFilter: mimeTypeFilters)
	{
		QMimeType mimeType = mimeDB.mimeTypeForName(mimeTypeFilter);

		if(mimeType.isValid())
		{
			allSupportedFormats.append(mimeType.globPatterns());
		}
	}

	QString allSupportedFormatsFilter = QString("All supported formats (%1)").arg(allSupportedFormats.join(' '));
	mimeTypeFilters.append(allSupportedFormatsFilter);

	dialog.setNameFilters(mimeTypeFilters);
	dialog.selectNameFilter(allSupportedFormatsFilter);

	if(acceptMode == QFileDialog::AcceptOpen)
		dialog.setFileMode(QFileDialog::ExistingFile);
	else
		dialog.setFileMode(QFileDialog::AnyFile);
}

static void initializeSpriteFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
	QStringList mimeTypeFilters = { "C1 (*.[sS][pP][rR])" , "C2 (*.[Ss]16)", "C2E (*.[Cc]16)", "All supported formats (*.[sS][pP][rR] *.[Ss]16 *.[Cc]16)" };

	dialog.setNameFilters(mimeTypeFilters);
	dialog.selectNameFilter(mimeTypeFilters.back());

	if(acceptMode == QFileDialog::AcceptOpen)
		dialog.setFileMode(QFileDialog::ExistingFile);
	else
		dialog.setFileMode(QFileDialog::AnyFile);
}

#endif
std::string MainWindow::GetImage()
{
	GLViewWidget::TimerState pause_timer(ui->viewWidget);

    QFileDialog dialog(this, tr("Open File"));

    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

#ifndef _WIN32
	dialog.setDirectory("/mnt/Passport/Programs/Cheetah-Texture-Packer/Cheeta-Texture-Packer/test-images");
#endif

	if(dialog.exec() == QDialog::Accepted)
	{
		return dialog.selectedFiles().first().toStdString();
	}

#if 0
    while(dialog.exec() == QDialog::Accepted)
	{
		QImageReader reader(dialog.selectedFiles().first());

		if(reader.format() == "png"
		|| reader.format() == "jpeg")
			return dialog.selectedFiles().first().toStdString();

		QFileInfo load_path = dialog.selectedFiles().first();
		QFileInfo save_path = dialog.selectedFiles().first() + ".png";

		if(save_path.exists()
		&& save_path.fileTime(QFile::FileTime::FileModificationTime) > load_path.fileTime(QFile::FileTime::FileModificationTime))
		{
			return save_path.path().toStdString();
		}

		if(QMessageBox::Yes == QMessageBox::question(this, "Opening File...", "Can only pack png/jpeg files. Convert to PNG?"))
		{
			QImage newImage = reader.read();

			if (newImage.isNull()) {
				throw std::runtime_error(
					QString("Cannot load %1: %2").arg(
						QDir::toNativeSeparators(dialog.selectedFiles().first()),
						reader.errorString()).toStdString());
			}

			QImageWriter writer(save_path.path());
			writer.write(newImage);

			return save_path.path().toStdString();
		}
	}
#endif

	return std::string();
}

std::string MainWindow::GetSpritePath()
{
	GLViewWidget::TimerState pause_timer(ui->viewWidget);

    QFileDialog dialog(this, tr("Open Sprite"));

    initializeSpriteFileDialog(dialog, QFileDialog::AcceptOpen);

#ifndef _WIN32
	dialog.setDirectory("/mnt/Passport/Programs/Cheetah-Texture-Packer/Cheeta-Texture-Packer/test-images");
#endif

    if(dialog.exec() == QDialog::Accepted)
	{
		return dialog.selectedFiles().first().toStdString();
	}

	return std::string();
}

void MainWindow::ImportSprite()
{
	std::string path;

	try
	{
		path = GetSpritePath();

		if(path.empty())
			return;

		auto image = Image::Factory(&document->imageManager, path);

		auto command  = std::make_unique<ObjectCommand>(document.get(), document->objects.size(), image->getFilename());
		auto material = command->GetObject().get()->material.get();
		material->ext.unlit.is_empty = false;
		material->SetImage(image, &material->image_slots[(int)Material::Tex::BaseColor]);
		material->Prepare(ui->viewWidget);

		document->addCommand(std::move(command));
	}
	catch(std::exception & e)
	{
		QMessageBox::warning(0, tr("Import failed"),
			tr("Problem opening file: ") + QString::fromStdString(path) + "\n" + e.what());
	}
}


bool MainWindow::selectTiles(int texture, QRect rect, Qt::KeyboardModifiers keys)
{
	/*
    bool selectionChanged = false;

    if(!(keys & Qt::ShiftModifier
    ||   keys & Qt::ControlModifier))
    {
        ui->tilesList->clearSelection();
        selectionChanged =  true;
    }

    for(uint32_t i = 0; i < packer.images.size(); ++i)
    {
        if((packer.images[i].duplicateId != NULL && packer.merge)
        ||  packer.images[i].textureId   != texture)
            continue;


        if(packer.images[i].getOutline(packer.cropThreshold).intersects(rect))
        {
            QListWidgetItem * item = packer.images[i].id->listItem;
            if(keys & Qt::ControlModifier)
            {
                selectionChanged =  true;
                item->setSelected(!item->isSelected());
            }
            else if(!item->isSelected())
            {
                selectionChanged =  true;
                item->setSelected(true);
            }
         }
    }

    return selectionChanged;
	*/
	return false;
}


void MainWindow::deleteSelectedTiles()
{/*
    for(int j = 0; j < packer.images.size(); ++j)
    {
        if(packer.images.at(j).id->listItem
        && packer.images.at(j).id->listItem->isSelected())
        {
            delete packer.images.at(j).id;
            packer.images.removeAt(j);
            --j;
        }
    }

    qDeleteAll(ui->tilesList->selectedItems());
	updateAuto();
	*/
}

void MainWindow::displayStatus(quint64 image_area, quint64 packer_area, quint64 neededArea, int missingImages, int mergedImages)
{
	QString message;

	if(image_area)
	{
		float percent =  packer_area * 100.0 / (float)image_area;
		float percent2 = neededArea  * 100.0 / (float)image_area;

		QString missed;
		QString merged;

		if(missingImages)
		{
			missed = tr("%1 images missed").arg(QString::number(missingImages));
			missed = QString(", <font color=red><b>%1</b></font>").arg(missed);
		}

		if(mergedImages)
		{
			merged  = tr(", %1 images merged").arg(mergedImages);
		}

		message = tr("%1% filled").arg(percent)
				+ missed
				+ merged
				+ tr(", needed area: %1%").arg(percent2)
				+ tr(", KBytes %3").arg(image_area * 4 / 1024);
	}

	m_status->setText(message);

}

void MainWindow::packerUpdate()
{
	/*
	PackerSettings settings;
	prefs->GetSettings(settings);

	if(packer.images.size())
	{
		packer.pack(settings);
		packerRepaint();
	}*/
}


void MainWindow::packerRepaint()
{
/*    bool previewWithImages = ui->c_previewWithImages->isChecked();

    QList<QImage> textures;
    for(int i = 0; i < packer.bins.size(); i++)
    {
        QImage texture(packer.bins.at(i).width(), packer.bins.at(i).height(),
                       QImage::Format_ARGB32);
        texture.fill(Qt::transparent);
        textures << texture;
    }
    if(exporting)
    {
        if(!WriteAtlas(0L, textures, packer, outDir, outFile, outFormat, imgFormat))
			return;
    }

    packer.CreateOutputTextures(textures, previewWithImages, exporting, &pattern);
    packer.DrawOutlines(textures);

	displayStatus(textures);

    if(exporting)
    {
        if(ExportImages(this, textures, outDir, outFile, imgFormat))
		{
			QMessageBox::information(0, tr("Done"), tr("Your atlas successfully saved in ") + outDir);
		}

        exporting = false;
    }
    else
    {
		m_view->updatePixmap(textures);
    }
	*/
}

void MainWindow::updateAplhaThreshold()
{
 //   packer.cropThreshold = prefs->alphaThreshold();
  //  packer.UpdateCrop();
   // updateAuto();
}
/*
void MainWindow::getFolder()
{
    ui->t_outDir->setText(QFileDialog::getExistingDirectory(this,
                        tr("Open Directory"),
                         ui->t_outDir->text(),
                        QFileDialog::ShowDirsOnly));
}*/

void MainWindow::exportImage()
{
    exporting = true;
    outDir = prefs->outDir();
    outFile = prefs->outFile();
    imgFormat = prefs->imgFormat();
    packerUpdate();
}

void MainWindow::clearTiles()
{
   // packer.images.clear();
   // ui->tilesList->clear();
}

void MainWindow::updateAuto()
{
    if(ui->c_autoUpdate->isChecked())
    {
        packerUpdate();
    }
}
/*
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> droppedUrls = event->mimeData()->urls();
    int droppedUrlCnt = droppedUrls.size();
    for(int i = 0; i < droppedUrlCnt; i++)
    {
        QString localPath = droppedUrls[i].toLocalFile();
        QFileInfo fileInfo(localPath);
        if(fileInfo.isFile())
        {
            ui->tilesList->addItem(fileInfo.fileName());
            packerData *data = new packerData;
            data->listItem = ui->tilesList->item(ui->tilesList->count() - 1);
            data->file = fileInfo.fileName();
			data->path = fileInfo.absoluteFilePath();
            packer.addItem(data->path, data);
            //QMessageBox::information(this, tr("Dropped file"), "Dropping files is not supported yet. Drag and drop directory here.");
        }
        else
            if(fileInfo.isDir())
            {
                addDir(fileInfo.absoluteFilePath());
            }
    }
    packerUpdate();

    event->acceptProposedAction();
}*/

float MainWindow::SetZoom(float zoom)
{
const float g_MinZoom = .125f;
const float g_MaxZoom = 2.f;

	float p_zoom = m_zoom;

	m_zoom = std::max(g_MinZoom, std::min(g_MaxZoom, zoom));

	ui->m_zoomIn->setEnabled(m_zoom < g_MaxZoom);
	ui->m_zoomIn->setEnabled(m_zoom > g_MinZoom);

	ui->horizontalScrollBar->setPageStep(m_zoom*64);
	ui->verticalScrollBar->setPageStep(m_zoom*64);

	ui->viewWidget->need_repaint(false);

	return p_zoom / m_zoom;
}

glm::vec2  MainWindow::GetScroll()
{
	return glm::vec2(
		(ui->horizontalScrollBar->value() - ui->horizontalScrollBar->minimum())
			/ (double) (ui->horizontalScrollBar->maximum() - ui->horizontalScrollBar->minimum()),
		 1 - (ui->verticalScrollBar->value() - ui->verticalScrollBar->minimum())
			/ (double) (ui->verticalScrollBar->maximum() - ui->verticalScrollBar->minimum()));
}

void  MainWindow::SetScroll(glm::vec2 scroll)
{
	scroll = glm::max(glm::vec2(0), glm::min(scroll, glm::vec2(1)));
	int scroll_x = glm::mix(ui->horizontalScrollBar->minimum(), ui->horizontalScrollBar->maximum(), scroll.x);
	int scroll_y = glm::mix(ui->verticalScrollBar->minimum(), ui->verticalScrollBar->maximum(), 1 - scroll.y);

	ui->horizontalScrollBar->setValue(scroll_x);
	ui->verticalScrollBar->setValue(scroll_y);
}
