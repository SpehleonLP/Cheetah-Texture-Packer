#include "preferences.h"
#include "imagemetadata.h"
#include "maxrects.h"
#include <algorithm>

namespace Preferences
{

const char * HeuristicStrings[] = {
	"None",
	"Top Left",
	"Best Area",
	"Best Short Side",
	"Best Long Side",
	"Min Width",
	"Min Height",
	"" };

const char * HeuristicToolTip[] = {
	"Weight function tries to find best place for each image. Top left will be best in most cases.",
	"Some heuristics depend on sorting method.",
	""
};

}

int MaxRects::CalculateHeuristic(glm::ivec4 itr, glm::ivec2 img, bool & leftNeighbor, bool & rightNeighbor) const
{
	switch(heuristic)
	{
	case Preferences::Heuristic::None: return 0;
	case Preferences::Heuristic::TopLeft:
	{
		int m = itr.y;
		for(uint32_t k = 0; k < R.size(); k++)
		{
			if(std::abs(R[k].y + (R[k].w - R[k].y) / 2 - (itr.w) / 2) <
					std::max(R[k].w - R[k].y, itr.w - itr.y) / 2)
			{
				if(R[k].w == itr.x)
				{
					m -= 5;
					leftNeighbor = true;
				}
				if(R[k].x == itr.z)
				{
					m -= 5;
					rightNeighbor = true;
				}
			}
		}

		if(leftNeighbor || rightNeighbor == false)
		{
			if(itr.z == size.x)
			{
				m -= 1;
				rightNeighbor = true;
			}
			if(itr.x == 0)
			{
				m -= 1;
				leftNeighbor = true;
			}
		}

		return m;
	} break;
	case Preferences::Heuristic::BestArea:      return (itr.z - itr.x) * (itr.w - itr.y);
	case Preferences::Heuristic::BestShortSide: return std::min((itr.z - itr.x) - img.x, (itr.w - itr.y) - img.y);
	case Preferences::Heuristic::BestLongSide:  return std::max((itr.z - itr.x) - img.x, (itr.w - itr.y) - img.y);
	case Preferences::Heuristic::MinWidth:      return itr.z - itr.x;
	case Preferences::Heuristic::MinHeight:     return itr.w - itr.y;
	default: break;
	}

	return 0;
}

namespace Preferences
{

const char * RotationStrings[] = {
	"Never",
	"Automatic",
	"H/2 < W < H",
	"W > H",
	"W > 2H",
	"W/2 < H < W",
	"H > W",
	"H > 2W",
""
};

const char * RotationToolTip[] = {
	"Heuristic for determining best rotation for each image, automatic is best in most cases.",
	""
};

bool ShouldRotate(Rotation rot, const glm::ivec2 & size)
{
	switch(rot)
	{
	case Rotation::H2_WIDTH_H:
		return  size.y >size.x &&size.x * 2 > size.y;
	case Rotation::WidthGtHeight:
		return size.x > size.y;
	case Rotation::WidthGt2Height:
		return size.x > 2 * size.y;
	case Rotation::W2_HEIGHT_W:
		return size.x > size.y && size.y * 2 >size.x;
	case Rotation::HeightGtWidth:
		return size.y >size.x;
	case Rotation::HeightGt2Width:
		return size.y > 2 *size.x;
	default:
		break;
	}

	return false;
}

const char * SortStrings[] =
{
	"None",
	"Width",
	"Height",
	"Area",
	"Max",
	""
};

const char * SortToolTip[] =
{
	""
};


static bool ImageCompareByHeight(const ImageMetadata &i1, const ImageMetadata &i2);
static bool ImageCompareByWidth(const ImageMetadata &i1, const ImageMetadata &i2);
static bool ImageCompareByArea(const ImageMetadata &i1, const ImageMetadata &i2);
static bool ImageCompareByMax(const ImageMetadata &i1, const ImageMetadata &i2);

SortFunction getSortFunction(Sort sort)
{
	switch(sort)
	{
	case Sort::Width:    return ImageCompareByWidth;
	case Sort::Height:   return ImageCompareByHeight;
	case Sort::Area: return ImageCompareByArea;
	case Sort::Max:  return ImageCompareByMax;
	default: break;
	}

	return 0L;
}

//all of these need to compare such that images with same w/h follow each other in array.

static bool ImageCompareByHeight(const ImageMetadata &i1, const ImageMetadata &i2)
{
    int first =  (i1.cropSize.y << 16) + i1.cropSize.x;
    int second = (i2.cropSize.y << 16) + i2.cropSize.x;

	if(first == second)
	{
		return i1.id < i2.id;
	}
	else
		return first > second;
}

static bool ImageCompareByWidth(const ImageMetadata &i1, const ImageMetadata &i2)
{
	int first =  (i1.cropSize.x << 16) + i1.cropSize.y;
    int second = (i2.cropSize.x << 16) + i2.cropSize.y;

	if(first == second)
		return i1.id < i2.id;
	else
		return first > second;
}

static bool ImageCompareByArea(const ImageMetadata &i1, const ImageMetadata &i2)
{
	int first =  i1.cropSize.x * i1.cropSize.y;
    int second = i2.cropSize.x * i2.cropSize.y;

	if(first == second)
		return i1.id < i2.id;
	else
		return first > second;
}

static bool ImageCompareByMax(const ImageMetadata &i1, const ImageMetadata &i2)
{
    int first = i1.cropSize.y > i1.cropSize.x ?
                (i1.cropSize.y << 16) + i1.cropSize.x :
				(i1.cropSize.x  << 16) + i1.cropSize.y;
    int second = i2.cropSize.y > i2.cropSize.x ?
				(i2.cropSize.y << 16) + i2.cropSize.x :
				(i2.cropSize.x  << 16) + i2.cropSize.y;

    if(first == second)
		return i1.id < i2.id;
    else
        return first > second;
}


const char * ScaleStrings[] =
{
	"Nearest",
	"Bilinear",
	"SuperXbr",
	""
};

const char * ScaleToolTip[] =
{
	""
};

#if 0

void g_scaleImage(QImage & image, Scale_t scaleType, float scaleFactor)
{
    if(scaleType == s_Nearest)
    {
        image = image.scaled(image.size()*scaleFactor, Qt::KeepAspectRatio, Qt::FastTransformation);
        return;
    }
    else if(scaleType == s_Bilinear)
    {
        image = image.scaled(image.size()*scaleFactor, Qt::KeepAspectRatio, Qt::FastTransformation);
        return;
    }

 //double until we exceed the goal size
    QSize finalSize = scaleFactor * image.size();

    for(;scaleFactor > 2; scaleFactor /= 2)
    {
        image = double_image(image);
    }

//then scale down bilinearly.
    if(scaleFactor != 1)
    {
        image = image.scaled(finalSize, Qt::KeepAspectRatio, Qt::FastTransformation);
    }
}

bool g_cacheScaled(Scale_t scaleType)
{
    return scaleType != s_Nearest;
}

#endif

const char * SizeConstraintStrings[] =
{
	"None",
	"Multiple of 4",
	"Power of two",
	""
};

const char  * SizeConstraintToolTip[] =
{
	""
};

#define rndPow2(v) (v? (int)(std::log2(v) +.5) : 0)

bool CanTypeConstraint(int constraint)
{
    return constraint <= 1;
}

int RoundConstrainedValue(int constraint, int v)
{
    switch(constraint)
    {
    case 0: return v;
    case 1: return (v + 2) & 0xFFffFFfc; break;
    case 2: return v? 1 << rndPow2(v) : 0; break;
    default: break;
    }

    return v;
}

bool TestConstrainedValue(int constraint, int v)
{
    switch(constraint)
    {
    case 0: return true;
    case 1: return v % 4 == 0;
    case 2: { float x = std::log2(v); return x == (int)x; }
    default: break;
    }

    return false;
}

int StepConstrainedValue(int constraint, int v, int step)
{
 //want it to just snap between 0 and 64
    if(step < 0 && v == 64) return -64;
    if(step > 0 && v ==  0) return  64;

    switch(constraint)
    {
    case 0: return step;
    case 1: return step * 4; break;
    case 2: return step < 0? -v/2 : v; break;
    default: break;
    }

    return step;
}

}
