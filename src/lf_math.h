#ifndef LF_MATH_H
#define LF_MATH_H
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <algorithm>

namespace math
{
	template<typename T, glm::qualifier Q>
	inline bool contains(glm::vec<4, T, Q> const& a, glm::vec<2, T, Q> const & b)
	{
		return a.x <= b.x && b.x < a.z
			&& a.y <= b.y && b.y < a.w;
	}

	template<typename T, glm::qualifier Q>
	inline bool contains(glm::vec<4, T, Q> const& a, glm::vec<4, T, Q> const & b)
	{
		return a.x <= b.x && b.x < a.z
			&& a.y <= b.y && b.y < a.w;
	}

	template<typename T, glm::qualifier Q>
	inline bool intersects(glm::vec<4, T, Q> const& a, glm::vec<4, T, Q> const & b)
	{
		return a.x < b.z && b.x < a.z
			&& a.y < b.w && b.x < a.w;
	}

	template<typename T>
	inline T min(T a, T b, T c ) { return std::min(a, std::min(b, c)); }
	template<typename T>
	inline T max(T a, T b, T c ) { return std::max(a, std::max(b, c)); }

	template<typename T>
	inline T min(T a, T b, T c, T d) { return std::min(std::min(a, b), std::min(c, d)); }
	template<typename T>
	inline T max(T a, T b, T c, T d) { return std::max(std::max(a, b), std::max(c, d)); }
};

#endif // LF_MATH_H
