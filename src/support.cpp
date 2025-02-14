#include "support.h"
#include "imagepacker.h"
#include <QImageWriter>
#include <QMessageBox>
#include <QDir>
#include <QTextStream>

QStringList g_extensions;

quint64 CalculateTotalArea(const QList<QImage> & textures)
{
	quint64 area = 0;

	for(int i = 0; i < textures.count(); i++)
	{
		area += textures.at(i).width() * textures.at(i).height();
	}

	return area;
}

bool ChangeDir(QWidget * parent, const QString & outDir)
{
	if(QDir::setCurrent(outDir))
		return true;

	WriteError(parent, QString("Unable to open directory \"%1\" for writing").arg(outDir));
	return false;
}

bool WriteAtlas(QWidget * parent, const QList<QImage> & textures, const ImagePacker & packer, const QString & outDir, const QString & outFile, QString outFormat, QString imgFormat)
{
	QString outputFile;
	QString imageFile;
	if(!ChangeDir(parent, outDir)) return false;
	outFormat = outFormat.toLower();

	for(int j = 0; j < textures.count(); j++)
	{
		if(textures.count() > 1)
		{
			imageFile  =  QString("%1-%2.%3").arg(outFile).arg(j+1).arg(imgFormat);
            outputFile =  QString("%1-%2.atlas.%3").arg(outFile).arg(j + 1).arg(outFormat);
		}
		else
		{
			imageFile  =  QString("%1.%2").arg(outFile).arg(imgFormat);
            outputFile =  QString("%1.atlas.%2").arg(outFile).arg(outFormat);
		}

		QFile positionsFile(outputFile);
		if(!positionsFile.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			WriteError(parent, QString("Cannot create file %1").arg(outputFile));
			return false;
		}
		else
		{
			QTextStream out(&positionsFile);
		//	packer.WriteAtlasFile(j, out, imageFile);
		}
	}

	return true;
}

bool ExportImages(QWidget * parent, const QList<QImage> & textures, const QString & outDir, const QString & outFile, const QString & outFormat)
{
	if(!ChangeDir(parent, outDir)) return false;

	QImageWriter writer;
	writer.setFormat(qPrintable(outFormat));
	writer.setQuality(100);

	for(int i = 0; i < textures.count(); i++)
	{
		if(textures.count() == 1)
		{
			writer.setFileName(QString("%1.%2").arg(outFile).arg(outFormat));
		}
		else
		{
			writer.setFileName(QString("%1_%2.%3").arg(outFile).arg(i+1).arg(outFormat));
		}

		if(!writer.write(textures[i]))
		{
			WriteError(parent, QString("Cannot create file %1").arg( writer.errorString()));
			return false;
		}
	}

	return 0;
}

void WriteError(QWidget * parent, const QString & string)
{
	if(parent)
	{
		QMessageBox::warning(parent, QCoreApplication::applicationName(), string);
	}
	else
	{
		fprintf(stderr, "%s", qPrintable(string));
	}
}

void printHelp(const char *error)
{
    if(error)
    {
        fputs(error, stderr);
    }
    printf(
        "Usage: cheetah-texture-packer [-s size] [-o OUTFILE] [options] [file|dir ...]\n"
        "Avaiable options:\n"
        "  -s, --size W[xH]           atlas maximum size (if it is not enough - create\n"
        "                             more than 1 atlas)\n"
        "  -o, --out-file OUTFILE     output atlas name\n"
        "      --disable-merge        do not merge similar images\n"
        "      --disable-crop         do not crop images\n"
        "      --crop-threshold N     crop threshold (0-255)\n"
        "      --disable-border       do not make 1px border\n"
        "      --border-size          set border size in pixels\n"
        "      --extrude-size         set extrude size in pixels\n"
        "      --enable-rotate        enable sprites rotation\n"
        "      --disable-recursion    disable recursive scan (pack only given directory)\n"
        "      --square               force to make square textures\n"
        "      --autosize-threshold N auto-optimize atlas size (0-100, 0 - disabled)\n"
        "      --min-texture-size WxH auto-optimize minimum size\n"
        "      --sort-order N         select sorting order algorithm (0-4)\n"
        "  -h, -?, --help             show this help and exit\n");
    if(error)
    {
        exit(1);
    }
    else
    {
        exit(0);
    }
}

void RecurseDirectory(ImagePacker & packer, const QString & topDir, const QString & dir, bool recursion)
{
    QDir dirEnt(dir);
    QFileInfoList list = dirEnt.entryInfoList();
    for(int i = 0; i < list.count(); i++)
    {
        QFileInfo info = list[i];

        QString filePath = info.filePath();
        QString fileExt = info.suffix().toLower();
        QString name = dir + QDir::separator();
        if(recursion && info.isDir())
        {
            // recursive
            if(info.fileName() != ".." && info.fileName() != ".")
            {
                RecurseDirectory(packer, topDir, filePath, recursion);
            }
        }
        else
            if(g_extensions.contains(fileExt))
            {
                if(!QFile::exists(name + info.completeBaseName() + QString(".atlas")))
                {
 /*                   packerData *data = new packerData();
                    data->path = info.absoluteFilePath();
                    data->file = info.fileName();
                    packer.addItem(data->path, data);*/
                }
            }
    }
}
