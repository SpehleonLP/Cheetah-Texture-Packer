#ifndef MAXRECTS_H
#define MAXRECTS_H
#include "preferences.h"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <vector>

struct ImageMetadata;

#ifdef NAIVE_IMPLEMENTATION
	for each new rect r
		add r to list

	for i in list 0...N
		for j in list (i+1)...N
			if(i.contains(j)) list.remove(j);
			if(j.contains(i)) list.remove(i);

   Assuming that we have a list of size N, and we are adding M new rectangels for it.
   This appears to be O((N+M)^2); but it isn't because removing from a dynamic array is O(N),
   So this implementation is O((N+M)^3)
#endif

class MaxRects
{
public:
	MaxRects(glm::ivec2 size, int border, glm::ivec2 align);

	glm::ivec2 insertNode(ImageMetadata &);
	int CalculateHeuristic(glm::ivec4 itr, glm::ivec2 img, bool & leftNeighbor, bool & rightNeighbor) const;

	inline int width()  const { return size.x / alignment.x; }
	inline int height() const { return size.y / alignment.y; }

	std::vector<glm::ivec4> F;
	std::vector<glm::ivec4> R;

	Preferences::Heuristic heuristic{Preferences::Heuristic::TopLeft};
	Preferences::Rotation rotation{Preferences::Rotation::Automatic};

	int16_t  padding{};

private:
	glm::ivec2 size;
	glm::ivec2 alignment;

//max rects works by dividing each rect that the placed object intersects with, into the largest resulting rectangles.
//we then try to find a placement within those rectangles.


    //this is much more optomized n*(m+n)
    //can be optimized much further, but that resulted in spaghetti code!
     static void CutIntersectingRects(std::vector<glm::ivec4> &list, glm::ivec4 buf, int padding);
     static void AddRect(std::vector<glm::ivec4> &list, glm::ivec4 rect);
     static int64_t sumArea(std::vector<glm::ivec4> &list);
};

#endif // MAXRECTS_H
