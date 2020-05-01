
#ifndef LAVOS_CAMERA_H
#define LAVOS_CAMERA_H

#include "component.h"
#include "transform_component.h"
#include "../node.h"

#include "../glm_config.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>

namespace lavos
{

class Camera: public Component
{
	public:
		enum class Type { PERSPECTIVE, ORTHOGRAPHIC } ;

	private:
		Type type = Type::PERSPECTIVE;

		glm::vec3 up_vector = glm::vec3(0.0f, 1.0f, 0.0f);

		float near_clip = 0.1f;
		float far_clip = 100.0f;

		float perspective_fovy = glm::radians(60.0f);
		float perspective_aspect = 1.0f;

		float orthographic_left = -1.0f;
		float orthographic_right = 1.0f;
		float orthographic_bottom = -1.0f;
		float orthographic_top = 1.0f;

	public:
		Type GetType() const 								{ return type; }
		void SetType(Type type)								{ this->type = type; }

		glm::vec3 GetUpVector() const						{ return up_vector; }
		void SetUpVector(const glm::vec3 &v)				{ up_vector = v; }

		float GetNearClip() const 							{ return near_clip; }
		void SetNearClip(float near_clip)					{ this->near_clip = near_clip; }

		float GetFarClip() const 							{ return far_clip; }
		void SetFarClip(float far_clip)						{ this->far_clip = far_clip; }

		float GetPerspectiveFovY() const 					{ return perspective_fovy; }
		void SetPerspectiveFovY(float fovy)					{ this->perspective_fovy = fovy; }

		float GetPerspectiveAspect() const					{ return perspective_aspect; }
		void SetPerspectiveAspect(float aspect)				{ perspective_aspect = aspect; }

		float GetOrthographicLeft() const 					{ return orthographic_left; }
		float GetOrthographicRight() const 					{ return orthographic_right; }
		float GetOrthographicBottom() const 				{ return orthographic_bottom; }
		float GetOrthographicTop() const 					{ return orthographic_top; }

		void SetPerspectiveParams(float fovy, float aspect)
			{ this->perspective_fovy = fovy; this->perspective_aspect = aspect; }

		void SetOrthographicParams(float left, float right, float bottom, float top)
			{ this->orthographic_left = left; this->orthographic_right = right; this->orthographic_bottom = bottom; this->orthographic_top = top; }


		glm::mat4 GetModelViewMatrix()
		{
			auto transform_component = GetNode()->GetTransformComp();
			if(transform_component == nullptr)
				throw std::runtime_error("node with a camera component does not have a transform component.");

			glm::mat4 transform_mat = transform_component->GetMatrixWorld();
			return glm::inverse(transform_mat);
		}

		glm::mat4 GetProjectionMatrix()
		{
			if(type == Type::PERSPECTIVE)
			{
				return glm::perspective(perspective_fovy, perspective_aspect, near_clip, far_clip);
			}
			else // ORTHOGRAPHIC
			{
				return glm::ortho(orthographic_left, orthographic_right, orthographic_bottom, orthographic_top,
								  near_clip, far_clip);
			}
		}
};

}

#endif //VULKAN_CAMERA_COMPONENT_H
