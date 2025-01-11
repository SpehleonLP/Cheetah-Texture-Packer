#pragma once

#include <ostream>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec4.hpp>

namespace std
{

template<int m, typename T, glm::qualifier Q>
inline std::string to_string(glm::vec<m, T, Q> const& it)
{
	std::string r = "vecT(";
	r[3] = '0' + m;

	for(int i = 0; i < m; ++i)
	{
		r += std::to_string(it[i]);

		if(i+1 < m)
			r += ", ";
		else
			r += ")";
	}

	return r;
}

template<int m, class T, glm::qualifier P>
inline std::ostream & operator<< (std::ostream & stream, const glm::vec<m, T, P> & db)
{
	return stream << std::to_string(db);
}

template<class T, glm::qualifier P>
inline std::ostream & operator<< (std::ostream & stream, const glm::tquat<T, P> & collider)
{
	stream << "quat(" << collider[0] << ", " << collider[1] << ", " << collider[2] << ", " << collider[3] << ")";
	return stream;
}


}
