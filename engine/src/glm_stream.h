
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

#endif //VULKAN_GLM_STREAM_H
