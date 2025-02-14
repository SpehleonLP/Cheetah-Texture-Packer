#ifndef SUPPORT_H
#define SUPPORT_H
#include <QStringList>
#include <QList>
#include <QImage>
#include <QString>

class QWidget;
class ImagePacker;
class QFileInfo;

extern QStringList g_extensions;

bool ChangeDir(QWidget * parent, const QString & outDir);
bool WriteAtlas(QWidget * parent, const QList<QImage> & textures, const ImagePacker & packer, const QString & outDir, const QString & outFile, QString outFormat, QString imgFormat);
bool ExportImages(QWidget * parent, const QList<QImage> & textures, const QString & outDir, const QString & outFile, const QString & outFormat);
void WriteError(QWidget * parent, const QString & string);
quint64 CalculateTotalArea(const QList<QImage> & textures);

void printHelp(const char *error = NULL);
void RecurseDirectory(ImagePacker & packer, const QString & topDir, const QString & dir, bool recursion);

#endif // SUPPORT_H
