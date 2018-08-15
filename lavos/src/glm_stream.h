
#ifndef VULKAN_GLM_STREAM_H
#define VULKAN_GLM_STREAM_H

#include "lavos/glm_config.h"
#include <glm/gtc/quaternion.hpp>
#include <ostream>

template<class T, glm::qualifier Q>
inline std::ostream& operator<<(std::ostream& stream, const glm::tvec2<T, Q> &v)
{
	return stream << "(" << v.x << ", " << v.y << ")";
}

template<class T, glm::qualifier Q>
inline std::ostream& operator<<(std::ostream& stream, const glm::tvec3<T, Q> &v)
{
	return stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

template<class T, glm::qualifier Q>
inline std::ostream& operator<<(std::ostream& stream, const glm::tvec4<T, Q> &v)
{
	return stream << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}

template<class T, glm::qualifier Q>
inline std::ostream& operator<<(std::ostream& stream, const glm::tquat<T, Q> &v)
{
	return stream << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}

template<class T, glm::qualifier Q>
inline std::ostream &operator<<(std::ostream &stream, const glm::tmat3x3<T, Q> &m)
{
	return stream << "(" << m[0].x << ",\t" << m[1].x << ",\t" << m[2].x << ",\n"
				  		 << m[0].y << ",\t" << m[1].y << ",\t" << m[2].y << ",\n"
				  		 << m[0].z << ",\t" << m[1].z << ",\t" << m[2].z << ")";
};

template<class T, glm::qualifier Q>
inline std::ostream &operator<<(std::ostream &stream, const glm::tmat4x4<T, Q> &m)
{
	return stream << "(" << m[0].x << ",\t" << m[1].x << ",\t" << m[2].x << ",\t" << m[3].x << ",\n"
				  		 << m[0].y << ",\t" << m[1].y << ",\t" << m[2].y << ",\t" << m[3].y << ",\n"
						 << m[0].z << ",\t" << m[1].z << ",\t" << m[2].z << ",\t" << m[3].z << ",\n"
						 << m[0].w << ",\t" << m[1].w << ",\t" << m[2].w << ",\t" << m[3].w << ")";
};

#endif //VULKAN_GLM_STREAM_H
