#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#   include <QtWidgets/QApplication>
#else
#   include <QtGui/QApplication>
#endif

#include "mainwindow.h"
#include "support.h"
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include <QDebug>
#include <QPainter>
#include "stdio.h"
#include "stdlib.h"
#include "parsearguments.h"
#include <loguru.hpp>
#include <cassert>
#include <cstdio>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <QImageWriter>

static
QStringList GetMimeTypes()
{
	QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = QImageWriter::supportedImageFormats();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();

	return mimeTypeFilters;
}


static void LogHandler(void* , const loguru::Message& message);

int main(int argc, char *argv[])
{
	// Put every log message in "everything.log":
	loguru::add_file("Log/status.log", loguru::Truncate, loguru::Verbosity_MAX);

	// Only log INFO, WARNING, ERROR and FATAL to "latest_readable.log":
	loguru::add_file("Log/error.log", loguru::Truncate, loguru::Verbosity_INFO);

	// Only show most relevant things on stderr:
	loguru::g_stderr_verbosity = 0;
//	loguru::add_callback("ERROR",&LogHandler,nullptr, loguru::Verbosity_ERROR);
//	loguru::set_fatal_handler(&FatalHandler);

    QApplication a(argc, argv);
	QCoreApplication::setApplicationName(QString("%1").arg(argv[0]).section('\\',-1));

	g_extensions = GetMimeTypes();

    //command-line version
    if(argc > 1)
    {
	/*	ImagePacker packer;
		Arguments args(argc, argv, packer);

		QString & outDir  = args.outDir;
        QString & outFile = args.outFile;

		qDebug() << "Saving to dir" << outDir << "and file" << outFile;

        Heuristic_t heuristic = TL;

        QString outFormat("PNG");

        if(packer.images.size() == 0)
        {
            fprintf(stderr, "No images found, exitting\n");
            exit(1);
        }

        packer.pack(heuristic, args.textureWidth, args.textureHeight);

        QList<QImage> textures;
        for(int i = 0; i < packer.bins.size(); i++)
        {
            QImage texture(packer.bins.at(i).width(), packer.bins.at(i).height(),
                           QImage::Format_ARGB32);
            texture.fill(Qt::transparent);
            textures << texture;
        }

        if(!WriteAtlas(0L, textures, packer, outDir, outFile, "txt", outFormat))
			return -1;

		packer.CreateOutputTextures(textures, false, true, 0L);

        qint64 area = CalculateTotalArea(textures);

        float percent = (((float)packerr .area / (float)area) * 100.0f);
        //        float percent2 = (float)(((float)packer.neededArea / (float)area) * 100.0f );
        printf("Atlas generated. %f%% filled, %d images missed, %d merged, %d KB\n",
               percent, packer.missingImages, packer.mergedImages, (int)((area * 4) / 1024));

        //        const char * format = qPrintable(outFormat);
		ExportImages(0L, textures, outDir, outFile, outFormat);

        return 0;
		*/
    }

    QTranslator myTranslator;
    myTranslator.load("cheetah_" + QLocale::system().name(), "qm");
    a.installTranslator(&myTranslator);
    MainWindow w;

	loguru::add_callback("WARNING",&LogHandler,&w, loguru::Verbosity_WARNING);

	int r_chdir{};

#ifdef _WIN32
    r_chdir = _chdir("F:/Programs/Cheetah-Texture-Packer/Cheeta-Texture-Packer/test-images");
#else
	r_chdir = chdir("/mnt/Passport/Programs/Cheetah-Texture-Packer/Cheeta-Texture-Packer/test-images");
#endif

	if(r_chdir) perror("problem setting working directory: ");

    w.show();


    return a.exec();
}

#include <QMessageBox>

void LogHandler(void* w, const loguru::Message& message)
{
	if(message.verbosity <= loguru::Verbosity_ERROR)
	{
		QMessageBox::critical((MainWindow*)w, QMainWindow::tr("Error"),
									  message.message);
	}
	else if(message.verbosity == loguru::Verbosity_WARNING)
	{
		QMessageBox::warning((MainWindow*)w, QMainWindow::tr("Warning"),
									  message.message);
	}
	else
	{
		QMessageBox::information((MainWindow*)w, QMainWindow::tr("Warning"),
									  message.message);
	}

	if(message.verbosity == loguru::Verbosity_ERROR)
	{
		((MainWindow*)w)->close();
	}

	if(message.verbosity == loguru::Verbosity_FATAL)
	{
		exit(-1);
	}
}
