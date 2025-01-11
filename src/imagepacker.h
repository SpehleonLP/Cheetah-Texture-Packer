#ifndef IMAGEPACKER_H
#define IMAGEPACKER_H
#include "imagemetadata.h"
#include "preferences.h"
#include <vector>

/*

class MaxRects;
class QTextStream;
class QListWidgetItem;

struct packerData
{
	packerData() : listItem(0L) {}
	QListWidgetItem *listItem;
	QString path;
	QString file;
};

struct packedImage
{
    QImage img;
//is this image itself part of an atlas?
    QRect originalSection;
    QRect crop;
    bool border, rotate;
    int textureId;
    int id;
};

struct ExtrudeData
{
    uint8_t t, b, l, r;
    QPoint pos() const { return QPoint(t, l); }
    int width()  const { return l+r; }
    int height() const { return t+b; }
};

struct inputImage
{
    quint32 hash;
    int textureId;
    packerData *id;
    inputImage *duplicateId;
    QPoint pos;
    QSize originalSize, sizeCurrent;
    QRect crop;
    QString path;

    QSize getSize(bool doCrop) const
    {
        return doCrop? crop.size() : originalSize;

    }

    QRect getCrop(bool doCrop) const
    {
       return doCrop? crop : QRect(0, 0, originalSize.width(), originalSize.height());
    //   return rotated? QRect(size.height() - r.y() - r.height(),  r.x(), r.height(), r.width()) : r;
    }

    QRect getOutline(bool doCrop)
    {
       QRect r = QRect(QPoint(0, 0), getSize(doCrop));
       r = rotated? QRect(r.y() - r.height(),  r.x(), r.height(), r.width()) : r;
	   return QRect(pos + r.topLeft(), r.size());
    }

    QImage getImage()
    {
        return id? QImage(id->path) : QImage();
    }

    bool cropped, rotated;
};

class ImagePacker
{
public:
        ImagePacker();

		void drawImage(QPainter & p, QImage & img, const QRect & crop);
		void WriteAtlasFile(int j, QTextStream & out, const QString & imgFile) const;
        void CreateOutputTextures(QList<QImage> & textures, bool preview, bool exporting, QPixmap * pattern);
		void DrawOutlines(QList<QImage> & textures);

        bool compareImages(QImage *img1, QImage *img2, int *i, int *j);
        void pack(Preferences::Heuristic heur, int w, int h);

        unsigned AddImgesToBins(Preferences::Heuristic heur, int w, int h);

        void CropLastImage(Preferences::Heuristic heur, int w, int h, bool wh);
        void DivideLastImage(Preferences::Heuristic heur, int w, int h, bool wh);

        void UpdateCrop();
		void applyGreenScreen(QImage & image);

        float GetFillRate();

        void ClearBin(int binIndex);

        int FillBin(Preferences::Heuristic heur, int w, int h, int binIndex);

        QRect crop(const QImage &img);
        ExtrudeData getExtrude(const QRect & crop);

        void addItem(const QImage &img, packerData *data, QString path);
        void addItem(QString path, packerData * data);
        const inputImage *find(void *data);
        void removeId(void *);
        void realculateDuplicates();
        void clear();

		std::vector<ImageMetadata> images;
		std::vector<glm::i16vec2> bins;

        int compare;
        quint64 area, neededArea;
        int missingImages;
        int mergedImages;


private:
	void internalPack(int heur, int w, int h);

	void SortImages(int w, int h);

};


*/

#endif // IMAGEPACKER_H
