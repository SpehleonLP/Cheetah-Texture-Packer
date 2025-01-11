#ifndef VECTOROPERATIONS_HPP
#define VECTOROPERATIONS_HPP
#include <glm/glm.hpp>
#include <vector>
#include "Support/shared_array.hpp"
#include "src/widgets/glviewwidget.h"

template<typename T, glm::qualifier Q>
struct VectorOperator : public std::vector<glm::vec<2, T, Q>>
{
typedef std::vector<glm::vec<2, T, Q>> super;
	typedef glm::vec<2, T, Q> (*Operation)(glm::vec<2, T, Q>);

	template<typename T2, glm::qualifier Q2>
	inline VectorOperator & Expand(glm::vec<4, T2, Q2> item)
	{
		push_back({item.x, item.y});
		push_back({item.z, item.y});
		push_back({item.z, item.w});
		push_back({item.x, item.w});

		return *this;
	}

	template<typename T2, glm::qualifier Q2>
	inline VectorOperator & Duplicate(glm::vec<2, T2, Q2> item)
	{
		push_back({item.x, item.y});
		push_back({item.x, item.y});
		push_back({item.x, item.y});
		push_back({item.x, item.y});

		return *this;
	}

	template<typename T2, glm::qualifier Q2>
	inline VectorOperator & Expand(glm::vec<4, T2, Q2> * array, uint32_t length)
	{
		for(uint32_t i = 0; i < length; ++i)
			Expand(array[i]);
	}

	template<typename T2, glm::qualifier Q2>
	inline VectorOperator & Expand(shared_array<glm::vec<4, T2, Q2> > & array)
	{
		return Expand(&array[0], array.size());
	}

	template<typename T2, glm::qualifier Q2>
	inline VectorOperator & Op(Operation Command)
	{
		for(auto & i : *this) i = Command(i);
	}

	inline VectorOperator & Upload(GLViewWidget * gl)
	{
		_gl glBufferData(GL_ARRAY_BUFFER, super::size() * sizeof(super::at(0)), &super::at(0), GL_STATIC_DRAW);
		return *this;
	}
};




#endif // VECTOROPERATIONS_HPP
