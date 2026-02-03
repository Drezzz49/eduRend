#include "Camera.h"

using namespace linalg;

void Camera::MoveTo(const vec3f& position) noexcept
{
	m_position = position;
}

void Camera::Move(const vec3f& direction) noexcept
{
	m_position += direction;
}

mat4f Camera::WorldToViewMatrix() const noexcept
{
	// Assuming a camera's position and rotation is defined by matrices T(p) and R,
	// the View-to-World transform is T(p)*R (for a first-person style camera).
	//
	// World-to-View then is the inverse of T(p)*R;
	//		inverse(T(p)*R) = inverse(R)*inverse(T(p)) = transpose(R)*T(-p)
	// Since now there is no rotation, this matrix is simply T(-p)

	mat4f rotation_matrix = mat4f::rotation(0.0f, yaw, pitch);  
	mat4f translation_matrix = mat4f::translation(-m_position); // Move the world the opposite way of the camera position

	return transpose(rotation_matrix) * translation_matrix;
}


void Camera::Rotate(float delta_yaw, float delta_pitch) noexcept
{
	yaw += delta_yaw; //lägg till förändringen i yaw
	pitch += delta_pitch; //lägg till förändringen i pitch

	// Clamp pitch to avoid gimbal lock
	const float limit = 1.5f; // ~85 degrees (cause 85 degrees ~ 1.5 radians)
	if (pitch > limit)
	{
		pitch = limit;
	}	
	else if (pitch < -limit)
	{
		pitch = -limit;
	}
}



mat4f Camera::ProjectionMatrix() const noexcept
{
	return mat4f::projection(m_vertical_fov, m_aspect_ratio, m_near_plane, m_far_plane);
}