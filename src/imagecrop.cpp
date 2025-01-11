#include "imagepacker.h"
#include "packersettings.h"
#include <QImage>

#define CMPF1(x,y,a) (qAlpha(img.pixel(x,y)) <= a)
#define CMPF3(x,y,a) (img.pixel(x,y) == a)

/*
#define CMPF1(x,y,a) (qAlpha(img.pixel(x,y)) > a)
*/

#ifdef _WIN32
#undef min
#undef max
#endif

//should be cached; is in the inputimage.cpp variation.
static bool doesImageHaveAlpha(const QImage & image);
#if 0
//auto-cropping algorithm
QRect crop(const QImage &img)
{
    int j, w, h, x, y;
    //QImage im;
    bool t;

	if(useGreenScreen && !doesImageHaveAlpha(img))
	{
		//crop only alpha
		t = true;
		for(y = 0; y < img.height(); y++)
		{
			for(j = 0; j < img.width(); j++)
			{
				t &= CMPF3(j, y, greenScreen);
				if(!t) break;
			}
			if(!t) break;
		}

		t = true;
		for(x = 0; x < img.width(); x++)
		{
			for(j = y; j < img.height(); j++)
			{
				t &= CMPF3(x, j, greenScreen);
				if(!t) break;
			}

			if(!t) break;
		}
		t = true;
		for(w = img.width(); w > 0; w--)
		{
			for(j = y; j < img.height(); j++)
			{
				t &= CMPF3(w - 1, j, greenScreen);
				if(!t) break;
			}

			if(!t) break;
		}
		t = true;
		for(h = img.height(); h > 0; h--)
		{
			for(j = x; j < w; j++)
			{
				t &= CMPF3(j, h - 1, greenScreen);
				if(!t) break;
			}

			if(!t) break;
		}
	}
	else
	{
		//crop only alpha
		t = true;
		for(y = 0; y < img.height(); y++)
		{
			for(j = 0; j < img.width(); j++)
			{
				t &= CMPF1(j, y, cropThreshold);
				if(!t) break;
			}

			if(!t) break;
		}

		t = true;
		for(x = 0; x < img.width(); x++)
		{
			for(j = y; j < img.height(); j++)
			{
				t &= CMPF1(x, j, cropThreshold);
				if(!t) break;
			}

			if(!t) break;
		}
		t = true;
		for(w = img.width(); w > 0; w--)
		{
			for(j = y; j < img.height(); j++)
			{
				t &= CMPF1(w - 1, j, cropThreshold);
				if(!t) break;
			}

			if(!t) break;
		}
		t = true;
		for(h = img.height(); h > 0; h--)
		{
			for(j = x; j < w; j++)
			{
				t &= CMPF1(j, h - 1, cropThreshold);
				if(!t) break;
			}

			if(!t) break;
		}
	}

    if(w < x) w = x;
    if(h < y) h = y;

    return QRect(x, y, w-x, h-y);
}
void ImagePacker::applyGreenScreen(QImage & image)
{
	if(!useGreenScreen
	|| !greenScreenToAlpha
	|| doesImageHaveAlpha(image)) return;

	for(int y = 0; y < image.height(); ++y)
	{
		for(int x = 0; x < image.width(); ++x)
		{
			QRgb px = image.pixel(x, y);
			if(px == greenScreen)
				image.setPixel(x, y, 0L);
		}
	}
}


static bool doesImageHaveAlpha(const QImage & image)
{
	if(!image.hasAlphaChannel()) return false;

	for(int y = 0; y < image.height(); ++y)
	{
		for(int x = 0; x < image.width(); ++x)
		{
			if(qAlpha(image.pixel(x, y)) < UCHAR_MAX)
				return true;
		}
	}

	return false;
}

#endif
