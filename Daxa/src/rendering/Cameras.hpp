#pragma once

#include "../DaxaCore.hpp"

#include "../platform/Window.hpp"

#define GLM_DEPTH_ZERO_TO_ONEW
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace daxa {
    
	class GimbalLockedCameraController {
	public:
		void processInput(daxa::Window& window, f32 dt);

		glm::mat4 getVP(daxa::Window& window) const;
	private:
		bool bZoom = false; 
		f32 fov = 74.0f;
		f32 near = 0.01f;
		f32 far = 1'000.0f;
		f32 cameraSwaySpeed = 0.0005f;
		f32 translationSpeed = 5.0f;
		glm::vec4 up = { 0.f, 0.f, 1.0f, 0.f };
		glm::vec4 position = { 0.f, -2.f, 0.f, 1.f };
		f32 yaw = 0.0f;
		f32 pitch = glm::pi<f32>() * 0.5f;
	};
}