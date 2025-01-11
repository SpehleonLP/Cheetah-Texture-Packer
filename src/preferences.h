#ifndef PREFERENCES_H
#define PREFERENCES_H
#include <glm/vec2.hpp>

struct ImageMetadata;

namespace Preferences
{

extern const char * HeuristicStrings[];
extern const char * HeuristicToolTip[];

enum class Heuristic : unsigned char
{
	None,
	TopLeft,
	BestArea,
	BestShortSide,
	BestLongSide,
	MinWidth,
	MinHeight,
	Total,
};

extern const char * RotationStrings[];
extern const char * RotationToolTip[];

enum class Rotation : unsigned char
{
	Never,
	Automatic,
	H2_WIDTH_H,
	WidthGtHeight,
	WidthGt2Height,
	W2_HEIGHT_W,
	HeightGtWidth,
	HeightGt2Width,
	Total
}; //rotation

bool ShouldRotate(Rotation rot, glm::ivec2 size);

extern const char * SortStrings[];
extern const char * SortToolTip[];

enum class Sort : unsigned char {
	None,
	Width,
	Height,
	Area,
	Max,
	Total
}; //sort

typedef bool (*SortFunction)(const ImageMetadata &i1, const ImageMetadata &i2);
SortFunction GetSortFunction(Sort sort);

extern const char * SizeConstraintStrings[];
extern const char * SizeConstraintToolTip[];

int RoundConstrainedValue(int constraint, int v);
int StepConstrainedValue(int constraint, int v, int step);
bool TestConstrainedValue(int constraint, int v);
bool CanTypeConstraint(int constraint);


};

#endif // PREFERENCES_H
