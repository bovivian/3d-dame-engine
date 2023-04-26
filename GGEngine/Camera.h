#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>

enum CAMERA_STATE
{
	CAMERA_STATE_FLY,
	CAMERA_STATE_ORBIT_PLAYER,
	CAMERA_STATE_LOOK_AT,
	CAMERA_STATE_MENU
};

class Camera
{
	private:
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::mat4 orthoMatrix;

		glm::vec3 position;
		glm::vec3 lookAt;
		glm::vec3 up;
		glm::vec3 direction;
		glm::vec3 orbitPoint;
		glm::vec3 orbitPointOrientation;
		float pitch, yaw, roll;
		float orbitRadius;
		float baseSpeed;
		float sensitivity;

		float fieldOfView;
		float aspectRatio;
		float nearClip;
		float farClip;

		CAMERA_STATE currentState;
	public:
		void Init();
		void HandleInput();
		void SetPosition(float x, float y, float z);
		void SetDirection(float x, float y, float z);
		void SetLookAt(float x, float y, float z);
		void SetPitch(float pitch);
		void SetYaw(float yaw);
		void SetOrbitParameters(glm::vec3 orbitPoint, glm::vec3 orbitPointOrientation, float radius);
		void SetCameraState(CAMERA_STATE state);
		glm::mat4 GetViewMatrix();
		glm::mat4 GetProjectionMatrix();
		glm::mat4 GetOrthoMatrix();
		glm::vec3 GetPosition();
		glm::vec3 GetDirection();
		CAMERA_STATE GetCameraState();
		float GetSensitivity();
		float GetFieldOfView();
		float GetAspectRatio();
		float GetNearClip();
		float GetFarClip();
};