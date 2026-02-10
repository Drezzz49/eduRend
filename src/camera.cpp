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

mat4f Camera::WorldToViewMatrix() const noexcept //kallas i render i scene.cpp och kameras position blir detta
{
	// Assuming a camera's position and rotation is defined by matrices T(p) and R,
	// the View-to-World transform is T(p)*R (for a first-person style camera).
	//
	// World-to-View then is the inverse of T(p)*R;
	//		inverse(T(p)*R) = inverse(R)*inverse(T(p)) = transpose(R)*T(-p)
	// Since now there is no rotation, this matrix is simply T(-p)



	// yaw är rotation runt y-axeln
	// pitch är rotation runt x-axeln

	//////////////////försök på egen förklaring
	// kameran har en pos i världen ex: (3, 2, 5), och den har en rotation åt den riktningen den tittar åt
	// vi vill flytta världen så att det ser ut som att kameran är i origo och tittar längs -z axeln
	// för att världen ska se ut som kamera är i 0,0,0 måste vi flytta världen -m_position
	// det gör vi genom att först rotera världen motsatt (transpose) mot kamerans rotation, sen flytta världen motsatt mot kamerans position

	mat4f rotation_matrix = mat4f::rotation(0.0f, yaw, pitch); //vår rotationsmatris
	mat4f translation_matrix = mat4f::translation(-m_position); //translationsmatrisen för världen


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



vec3f Camera::GetForward() const noexcept
{
	return normalize(vec3f(
		-sin(yaw) * cos(pitch),
		sin(pitch),
		-cos(yaw) * cos(pitch)
	));
}

vec3f Camera::GetRight() const noexcept
{
	vec3f forward = GetForward();
	vec3f world_up = vec3f(0.0f, 1.0f, 0.0f);

	return normalize(forward % world_up);
}

vec3f Camera::GetUp() const noexcept
{
	return normalize(GetRight() % GetForward());
}

mat4f Camera::ProjectionMatrix() const noexcept
{
	return mat4f::projection(m_vertical_fov, m_aspect_ratio, m_near_plane, m_far_plane);
}