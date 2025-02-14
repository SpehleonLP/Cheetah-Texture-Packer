#include "maxrects.h"
#include "imagemetadata.h"
#include "lf_math.h"
#include <iostream>
#include <climits>
#include <cassert>

MaxRects::MaxRects(glm::ivec2 size, int border, glm::ivec2 align) :
	size(size),
	alignment(align)
{
    F.push_back(glm::ivec4(border, border, size.x-border*2, size.y-border*2));
}

glm::ivec2 MaxRects::insertNode(ImageMetadata & input)
{
    int min = INT_MAX, mini = -1;
    glm::ivec2 img = input.currentSize;

    //    if(img.width() == w) img.setWidth(img.width() - border->l - border->r);
    //    if(img.height() == h) img.setHeight(img.height() - border->t - border->b);
    if(img.x <= 1 || img.y <= 1)
    {
        return glm::ivec2(-1, -1);
    }

    bool leftNeighbor = false, rightNeighbor = false;
    bool _leftNeighbor = false, _rightNeighbor = false;
    bool bestIsRotated = false;

    for(uint32_t i = 0; i < F.size(); i++)
    {
        glm::ivec4 itr = F[i];

//check each image with and without rotation
		bool rotated = false;

		for(;;)
		{
            //can we fit in this hole?
            if(itr.z - itr.x >= img.x && itr.w - itr.y >= img.y)
            {
                const int m = CalculateHeuristic(itr, img, _leftNeighbor, _rightNeighbor);

                if(m < min)
                {
                    min = m;
                    mini = i;
                    leftNeighbor = _leftNeighbor;
                    rightNeighbor = _rightNeighbor;
                    bestIsRotated = rotated;
                }
           }

            if(rotated || rotation == Preferences::Rotation::Never) break;
			rotated = true;
			std::swap(img.x, img.y);
		}

		if(rotated)
		{
			std::swap(img.x, img.y);
		}
    }

    if(mini < 0)
		return glm::ivec2(-1, -1);

    if(bestIsRotated)
    {
		std::swap(img.x, img.y);
		std::swap(input.currentSize.x, input.currentSize.y);
        input.rotated = !input.rotated;
    }

    glm::ivec4 buf(F[mini].x, F[mini].y, F[mini].x + img.x, F[mini].y + img.y);

// this was in the original source code by github.com/scriptum
// i really have no idea what it does.
	if(heuristic == Preferences::Heuristic::TopLeft)
	{
		if(!leftNeighbor && F[mini].x != 0 && F[mini].z == size.x)
		{
			buf = glm::ivec4(size.x - img.x, F[mini].y, img.x, img.y);
		}
		if(!leftNeighbor && rightNeighbor)
		{
			buf = glm::ivec4(F[mini].z - img.x, F[mini].y, img.x, img.y);
		}
    }

    R.push_back(buf);

    CutIntersectingRects(F, buf, padding);

	return glm::ivec2(buf.x, buf.y);
}

void MaxRects::CutIntersectingRects(std::vector<glm::ivec4> &list, glm::ivec4 buf, int padding)
{
 //add padding to buf (can't do this beforehand)
    if(padding)
        buf = glm::ivec4(buf.x-padding, buf.y-padding, buf.z+padding, buf.w+padding);

    for(uint32_t i = 0; i < list.size(); i++)
	{
        glm::ivec4 itr = list[i];

		if(!math::intersects(itr, buf))
			continue;

        list.erase(list.begin()+i); --i;

		if(buf.z < itr.z)
            AddRect(list, glm::ivec4(buf.z, itr.y, itr.z, itr.w));

		if(buf.w < itr.w)
            AddRect(list,
               glm::ivec4(itr.x, buf.w, itr.z, itr.w));

		if(buf.x > itr.x)
            AddRect(list, glm::ivec4(itr.x, itr.y, buf.x, itr.w));

		if(buf.y > itr.y)
            AddRect(list, glm::ivec4(itr.x, itr.y, itr.z, buf.y));
    }
}


void MaxRects::AddRect(std::vector<glm::ivec4> & list, glm::ivec4 rect)
{
//r = read, w = write
    uint32_t r,w;

    for(r=w=0; r < list.size(); ++r)
    {
        if(math::contains(list[r], rect))
        {
              assert(r == w);
              return;
        }

    // fundmentally, the issue is that we don't want to remove anything;
    // each removal is an O(N) operation in a vector.
    // so we want to shuffle everything along at once, then cut off the tail.
        if(!math::contains(rect, list[r]))
        {
            if(w != r) list[w] = list[r];
            ++w;
        }
	}

//shouldn't dealloc memory, just reduce the end ptr.
    list.resize(w);
    list.push_back(rect);
}


int64_t MaxRects::sumArea(std::vector<glm::ivec4> &list)
{
    int64_t area = 0;
    for(uint32_t i = 0; i < list.size(); ++i)
        area += (list[i].z - list[i].x)* (list[i].w - list[i].y);

    return area;
}
