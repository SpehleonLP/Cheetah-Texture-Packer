typedef int nothing;
#if 0
#include "rc_crc32.h"
#include "imagepacker.h"
#include <QDebug>
#include "maxrects.h"
#include <cassert>

ImagePacker::ImagePacker()
{
    extrude = 1;
	alignment = QPoint(4, 4);
    cropThreshold = 10;
    minTextureSizeX = 32;
    minTextureSizeY = 32;
	greenScreen = qRgba(0, 0, 0, 255);
	greenScreenToAlpha = true;
	useGreenScreen = true;
}

void ImagePacker::pack(Preferences::Heuristic heur, int w, int h)
{
// alignment below 1 px limit doesn't make any sense!
	assert(alignment.x() > 0 && alignment.y() > 0);

    SortImages(w, h);

    missingImages = 1;
    mergedImages = 0;
    area = 0;
    bins.clear();

    unsigned areaBuf = AddImgesToBins(heur, w, h);

    if(areaBuf && !missingImages)
    {
        CropLastImage(heur, w, h, false);
    }

    if(merge)
        for(int i = 0; i < images.size(); i++)
		{
			inputImage & img = images[i];
            if(img.duplicateId != NULL)
            {
				const inputImage * value = img.duplicateId;
                img.pos       = value->pos;
                img.textureId = value->textureId;
                mergedImages++;
            }
		}
}

void ImagePacker::UpdateCrop()
{
    for(int i = 0; i < images.size(); ++i)
    {
        images[i].crop = crop(QImage(images[i].path));
		images[i].sizeCurrent = images[i].crop.size();
    }
}

void ImagePacker::addItem(const QImage &img, packerData * data, QString path)
{
    inputImage i;
    if(img.width() == 0 || img.height() == 0)
    {
        return;
    }
    i.hash = rc_crc32(0, img.bits(), img.byteCount());
    i.crop = crop(img);
	i.sizeCurrent = i.crop.size();
    i.originalSize = img.size();
    i.id = data;
    i.path = path;
    images << i;
}

void ImagePacker::addItem(QString path, packerData *data)
{
    addItem(QImage(path), data, path);
}

void ImagePacker::clear()
{
    images.clear();
}

void ImagePacker::realculateDuplicates()
{
    for(int i = 0; i < images.count(); i++)
    {
        images[i].duplicateId = NULL;
    }
    for(int i = 0; i < images.count(); i++)
    {
//call stack operation & dual pointer lookup can be avoided
		inputImage & i0 = images[i];

        for(int k = i + 1; k < images.count(); k++)
        {
			inputImage & k0 = images[k];

            if(k0.duplicateId == NULL &&
                    i0.hash == k0.hash &&
                    i0.originalSize == k0.originalSize &&
                    i0.crop == k0.crop)
            {
                k0.duplicateId = &i0;
            }
        }
    }
}

void ImagePacker::SortImages(int w, int h)
{
    realculateDuplicates();
    neededArea = 0;
    QSize size;
    for(int i = 0; i < images.size(); i++)
    {
		inputImage & img = images[i];

        img.pos = QPoint(-1, -1);
		size = img.getSize(cropThreshold);

        if(size.width() != w)
        {
            size.setWidth(size.width() + 2 * extrude);
        }
        if(size.height() != h)
        {
            size.setHeight(size.height() + 2 * extrude);
        }


		if(alignment.x() && size.width() % alignment.x())
		{
			size.setWidth(size.width() + (alignment.x() - (size.width() % alignment.x())));
		}
		if(alignment.y() && size.height() % alignment.y())
		{
			size.setHeight(size.height() + (alignment.y() - (size.height() % alignment.y())));
        }


        img.rotated = false;
        if(shouldRotate(rotate, size))
        {
            size.transpose();
            img.rotated = true;
        }
        img.sizeCurrent = size;
        if(img.duplicateId == NULL || !merge)
        {
            neededArea += size.width() * size.height();
        }
    }

	SortFunction_t func = getSortFunction(sortOrder);
	if(func)
	{
		qStableSort(images.begin(), images.end(), func);
	}
}

int ImagePacker::FillBin(Heuristic_t heur, int w, int h, int binIndex)
{
    int areaBuf = 0;
    MaxRects rects(w, h, border, alignment);
    rects.heuristic = heur;
    rects.rotation = rotate;
    rects.padding  = padding;

    for(int i = 0; i < images.size(); i++)
    {
		inputImage & img = images[i];

        if(QPoint(-1, -1) != img.pos)
        {
            continue;
        }
        if( img.duplicateId == NULL || !merge)
        {
            img.pos = rects.insertNode(&img);
            img.textureId = binIndex;
            if(QPoint(-1, -1) == img.pos)
            {
                missingImages++;
            }
            else
            {
//compiler can't optimize this (doesn't know that these values aren't changed in another thread)
				float eax = img.sizeCurrent.width() * img.sizeCurrent.height();
                areaBuf += eax;
                area    += eax;
            }
        }
    }
    return areaBuf;
}

void ImagePacker::ClearBin(int binIndex)
{
    for(int i = 0; i < images.size(); i++)
    {
		inputImage & img = images[i];

        if( img.textureId == binIndex)
        {
            area -= img.sizeCurrent.width() * img.sizeCurrent.height();
            img.pos = QPoint(-1, -1);
        }
    }
}

unsigned ImagePacker::AddImgesToBins(Heuristic_t heur, int w, int h)
{
    int binIndex = bins.count() - 1;
    unsigned areaBuf = 0;
    unsigned lastAreaBuf = 0;
    do
    {
        missingImages = 0;
        bins << QSize(w, h);
        lastAreaBuf = FillBin(heur, w , h , ++binIndex);
        if(!lastAreaBuf)
        {
            bins.removeLast();
        }
        areaBuf += lastAreaBuf;
    }
    while(missingImages && lastAreaBuf);
    return areaBuf;
}

void ImagePacker::CropLastImage(Heuristic_t heur, int w, int h, bool wh)
{
    missingImages = 0;
    QList<inputImage> last_images = images;
    QList<QSize> last_bins = bins;
    quint64 last_area = area;

    bins.removeLast();
    ClearBin(bins.count());

    if(square)
    {
        w /= 2;
        h /= 2;
    }
    else
    {
        if(wh)
        {
            w /= 2;
        }
        else
        {
            h /= 2;
        }
        wh = !wh;
    }

    int binIndex = bins.count();
    missingImages = 0;
    bins << QSize(w, h);
    FillBin(heur, w , h , binIndex);
    if(missingImages)
    {
        images = last_images;
        bins = last_bins;
        area = last_area;
        missingImages = 0;
        if(square)
        {
            w *= 2;
            h *= 2;
        }
        else
        {
            if(!wh)
            {
                w *= 2;
            }
            else
            {
                h *= 2;
            }
            wh = !wh;
        }
        if(autosize)
        {
            float rate = GetFillRate();
            if((rate < (static_cast<float>(minFillRate) / 100.f)) &&
                    ((w > minTextureSizeX) && (h > minTextureSizeY)))
            {
                DivideLastImage(heur, w, h, wh);
                if(GetFillRate() <= rate)
                {
                    images = last_images;
                    bins = last_bins;
                    area = last_area;
                }
            }
        }
    }
    else
    {
        CropLastImage(heur, w, h, wh);
    }
}

void ImagePacker::DivideLastImage(Heuristic_t heur, int w, int h, bool wh)
{
    missingImages = 0;
    QList<inputImage> last_images = images;
    QList<QSize> last_bins = bins;
    quint64 last_area = area;

    bins.removeLast();
    ClearBin(bins.count());

    if(square)
    {
        w /= 2;
        h /= 2;
    }
    else
    {
        if(wh)
        {
            w /= 2;
        }
        else
        {
            h /= 2;
        }
        wh = !wh;
    }
    AddImgesToBins(heur, w, h);
    if(missingImages)
    {
        images = last_images;
        bins = last_bins;
        area = last_area;
        missingImages = 0;
    }
    else
    {
        CropLastImage(heur, w, h, wh);
    }
}

float ImagePacker::GetFillRate()
{
    quint64 binArea = 0;
    for(uint32_t i = 0; i < bins.size(); i++)
    {
        binArea += (bins[i].z - bins[i].x) * bins.at(i).height();
    }
    return (float)((double)area / (double)binArea);
}

#include <QTextStream>
#include <QPainter>

void ImagePacker::WriteAtlasFile(int j, QTextStream & out, const QString & imgFile) const
{
	out << "textures: " << imgFile << "\n";
	for(int i = 0; i < images.size(); i++)
	{
		const inputImage & img = images[i];
		if(img.textureId != j)
		{
			continue;
		}
        QPoint pos(img.pos.x() + extrude, img.pos.y() + extrude);
		QSize sizeOrig = img.originalSize;
		QSize size = img.getSize(cropThreshold);
		QRect crop = img.getCrop(cropThreshold);

		if(img.rotated)
		{
			size.transpose();
			crop = QRect(crop.y(), crop.x(), crop.height(), crop.width());
		}

		out << images[i].id->file
			<< "\t" << pos.x()
		    << "\t" << pos.y()
			<< "\t" << sizeOrig.width()
			<< "\t" << sizeOrig.height()

			<< "\t" << crop.x()
			<< "\t" << crop.y()
			<< "\t" << crop.width()
			<< "\t" << crop.height()

			<< "\t" << (img.rotated ? "r" : "")
			<< "\n";
	}
}

#include <QListWidgetItem>

void ImagePacker::CreateOutputTextures(QList<QImage> & textures, bool preview, bool exporting, QPixmap * pattern)
{
    for(int i = 0; i < images.size(); i++)
	{
		inputImage & itr = images[i];

    //	qDebug() << "Processing" << images[i].id->file;

		if(itr.duplicateId != NULL && merge)
		{
			continue;
        }

        QSize size = itr.getSize(cropThreshold);
        QRect crop = itr.getCrop(cropThreshold);
        QImage img = itr.getImage();
		applyGreenScreen(img);

		if(itr.textureId < bins.size())
		{
			QPainter p(&textures[itr.textureId]);
            p.translate(itr.pos);
            if(itr.rotated)
            {
                p.translate(size.height(), 0);
                p.rotate(90);
            }

//background chekerboard, could be moved to display widget?
			if(!exporting)
            {
                ExtrudeData extrude = getExtrude(crop);
                p.fillRect(0, 0, size.width() + extrude.width(),
                           size.height() + extrude.height(), *pattern);
            }

			if(preview || exporting)
            {
				drawImage(p, img, crop);
            }
            else if(!exporting)
			{
				p.drawRect(0, 0, size.width() - 1, size.height() - 1);
			}
		}
    }
}


void ImagePacker::DrawOutlines(QList<QImage> & textures)
{
	for(int i = 0; i < images.size(); i++)
	{
		inputImage * itr           = &images[i];
		QListWidgetItem * listItem = itr->id->listItem;

		if(listItem == 0L) continue;

		if(itr->pos != QPoint(-1, -1))
			listItem->setForeground(Qt::black);
		else
		{
		    listItem->setForeground(Qt::red);
			continue;
		}

		if(itr->duplicateId != NULL && merge)
		{
			continue;
		}

		if(!listItem->isSelected()) continue;

        QRect crop = itr->getOutline(cropThreshold);

		if(itr->textureId < bins.size())
		{
            QPainter p(&textures[itr->textureId]);
			p.setPen(Qt::yellow);
			//p.setBrush(Qt::NoBrush);

            ExtrudeData extrude = getExtrude(crop);
            p.drawRect(extrude.l+crop.x(), extrude.t+crop.y(), crop.width(), crop.height());
		}
    }
}

ExtrudeData ImagePacker::getExtrude(const QRect & crop)
{
    ExtrudeData ext = {0, 0, 0, 0};

	if(alignment.x())
	{
        ext.r = (crop.width()  + 2*extrude) % alignment.x();
        if(ext.r) ext.r = alignment.x() - ext.r;
	}
	if(alignment.y())
	{
        ext.b = (crop.height() + 2*extrude) % alignment.y();
        if(ext.b) ext.b = alignment.y() - ext.b;
	}

    ext.r += 2*extrude;
    ext.b += 2*extrude;

//compression gets better quality if more px in block of 4 are the same, so distribute it.
    ext.l  = ext.r >> 1; ext.r -= ext.l;
    ext.t  = ext.b >> 1; ext.b -= ext.t;

    return ext;
}


static
void drawCorner(QPainter & p, QColor c, const QRect & target)
{
	assert(target.width() >= 0 && target.height() >= 0);

	p.setPen(c);
	p.setBrush(c);

	if(target.width() && target.height())
		p.drawRect(target);
	else if(target.width())
		p.drawLine(target.x(), target.y(), target.right()+1, target.y());
	else if(target.height())
		p.drawLine(target.x(), target.y(), target.x(), target.bottom()+1);
	else
		p.drawPoint(target.topLeft());
}


void ImagePacker::drawImage(QPainter & p, QImage & img, const QRect & crop)
{
//	applyGreenScreen(img);
    ExtrudeData extrude = getExtrude(crop);

//top left
    if(extrude.t && extrude.l)
	{
        QRect target = QRect(0, 0, extrude.l - 1, extrude.t - 1);
		QColor c = QColor::fromRgba(img.pixel(crop.x(), crop.y()));
		drawCorner(p, c, target);
	}

//bottom left
    if(extrude.l && extrude.b)
	{
        QRect target = QRect(0, crop.height() + extrude.t, extrude.l - 1, extrude.b - 1);
		QColor c = QColor::fromRgba(img.pixel(crop.x(), crop.bottom()));
		drawCorner(p, c, target);
	}

//top right
    if(extrude.t && extrude.r)
	{
        QRect target = QRect(crop.width() + extrude.l, 0, extrude.r - 1, extrude.t - 1);
		QColor c = QColor::fromRgba(img.pixel(crop.right(), crop.y()));
		drawCorner(p, c, target);
	}

//bottom right
    if(extrude.b && extrude.r)
	{
        QRect target = QRect(crop.width() + extrude.l, crop.height() + extrude.t, extrude.r - 1, extrude.b - 1);
		QColor c = QColor::fromRgba(img.pixel(crop.right(), crop.bottom()));
		drawCorner(p, c, target);
	}

//left edge
    if(extrude.l)
	{
        QRect target = QRect(0, extrude.t, extrude.l, crop.height());
		QRect source = QRect(crop.x(), crop.y(), 1, crop.height());

		p.drawImage(target, img, source);
	}

//right edge
    if(extrude.r)
	{
        QRect target = QRect(crop.width() + extrude.l, extrude.t, extrude.r, crop.height());
		QRect source = QRect(crop.right(), crop.y(), 1, crop.height());

		p.drawImage(target, img, source);
	}

//top edge
    if(extrude.t)
	{
        QRect target = QRect(extrude.l, 0, crop.width(), extrude.t);
		QRect source = QRect(crop.x(), crop.y(), crop.width(), 1);

		p.drawImage(target, img, source);
	}
//bottom edge
    if(extrude.b)
	{
        QRect target = QRect(extrude.l, crop.height() + extrude.t, crop.width(), extrude.b);
		QRect source = QRect(crop.x(), crop.bottom(), crop.width(), 1);

		p.drawImage(target, img, source);
	}


    p.drawImage(extrude.l, extrude.t, img, crop.x(), crop.y(), crop.width(), crop.height());
}


#endif
