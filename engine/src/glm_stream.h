
#ifndef VULKAN_GLM_STREAM_H
#define VULKAN_GLM_STREAM_H

#include "glm_config.h"
#include <glm/gtc/quaternion.hpp>
#include <ostream>

inline std::ostream& operator<<(std::ostream& stream, const glm::vec2 &v)
{
	return stream << "(" << v.x << ", " << v.y << ")";
}

inline std::ostream& operator<<(std::ostream& stream, const glm::vec3 &v)
{
	return stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

inline std::ostream& operator<<(std::ostream& stream, const glm::vec4 &v)
{
	return stream << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}

inline std::ostream& operator<<(std::ostream& stream, const glm::quat &v)
{
	return stream << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}

template<class T, glm::precision P>
inline std::ostream &operator<<(std::ostream &stream, const glm::tmat3x3<T, P> &m)
{
	return stream << "(" << m[0].x << ",\t" << m[1].x << ",\t" << m[2].x << ",\n"
				  		 << m[0].y << ",\t" << m[1].y << ",\t" << m[2].y << ",\n"
				  		 << m[0].z << ",\t" << m[1].z << ",\t" << m[2].z << ")";
};

template<class T, glm::precision P>
inline std::ostream &operator<<(std::ostream &stream, const glm::tmat4x4<T, P> &m)
{
	return stream << "(" << m[0].x << ",\t" << m[1].x << ",\t" << m[2].x << ",\t" << m[3].x << ",\n"
				  		 << m[0].y << ",\t" << m[1].y << ",\t" << m[2].y << ",\t" << m[3].y << ",\n"
						 << m[0].z << ",\t" << m[1].z << ",\t" << m[2].z << ",\t" << m[3].z << ",\n"
						 << m[0].w << ",\t" << m[1].w << ",\t" << m[2].w << ",\t" << m[3].w << ")";
};

#endif //VULKAN_GLM_STREAM_H
