#include "Cameras.hpp"

namespace daxa {
    void GimbalLockedCameraController::processInput(daxa::Window& window, f32 dt) {
		f32 speed = window.keyPressed(GLFW_KEY_LEFT_SHIFT) ? translationSpeed * 4.0f : translationSpeed;
		if (window.isCursorCaptured()) {
			if (window.keyJustPressed(GLFW_KEY_ESCAPE)) {
				window.releaseCursor();
			}
		} else {
			if (window.buttonJustPressed(GLFW_MOUSE_BUTTON_LEFT) && window.isCursorOverWindow()) {
				window.captureCursor();
			}
		}

		auto cameraSwaySpeed = this->cameraSwaySpeed;
		if (window.keyPressed(GLFW_KEY_C)) {
			cameraSwaySpeed *= 0.25;
			bZoom = true;
		} else {
			bZoom = false;
		}

		auto yawRotaAroundUp = glm::rotate(glm::mat4(1.0f), yaw, {0.f,0.f,1.f});
		auto pitchRotation = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3{1.f,0.f,0.f});
		glm::vec4 translation = {};
		if (window.keyPressed(GLFW_KEY_W)) {
			glm::vec4 direction = { 0.0f, 0.0f, -1.0f, 0.0f };
			translation += yawRotaAroundUp * pitchRotation * direction * dt * speed;
		}
		if (window.keyPressed(GLFW_KEY_S)) {
			glm::vec4 direction = { 0.0f, 0.0f, 1.0f, 0.0f };
			translation += yawRotaAroundUp * pitchRotation * direction * dt * speed;
		}
		if (window.keyPressed(GLFW_KEY_A)) {
			glm::vec4 direction = { 1.0f, 0.0f, 0.0f, 0.0f };
			translation += yawRotaAroundUp * direction * dt * speed;
		}
		if (window.keyPressed(GLFW_KEY_D)) {
			glm::vec4 direction = { -1.0f, 0.0f, 0.0f, 0.0f };
			translation += yawRotaAroundUp * direction * dt * speed;
		}
		if (window.keyPressed(GLFW_KEY_SPACE)) {
			translation += yawRotaAroundUp * pitchRotation * glm::vec4{ 0.f,  1.f, 0.f, 0.f } * dt * speed;
		}
		if (window.keyPressed(GLFW_KEY_LEFT_CONTROL)) {
			translation += yawRotaAroundUp * pitchRotation * glm::vec4{ 0.f, -1.f,  0.f, 0.f } * dt * speed;
		}
		if (window.isCursorCaptured()) {
			pitch -= window.getCursorPosChangeY() * cameraSwaySpeed * dt;
			pitch = std::clamp(pitch, 0.0f, glm::pi<f32>());
			yaw += window.getCursorPosChangeX() * cameraSwaySpeed * dt;
		}
		position += translation;
	}

	glm::mat4 GimbalLockedCameraController::getVP(daxa::Window& window) const {
		auto fov = this->fov;
		if (bZoom) {
			fov *= 0.25f;
		}
		auto yawRotaAroundUp = glm::rotate(glm::mat4(1.0f), yaw, {0.f,0.f,1.f});
		auto pitchRotation = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3{1.f,0.f,0.f});
		auto prespective = glm::perspective(fov, (f32)window.getWidth()/(f32)window.getHeight(), near, far);
		auto rota = yawRotaAroundUp * pitchRotation;
		auto cameraModelMat = glm::translate(glm::mat4(1.0f), {position.x, position.y, position.z}) * rota;
		auto view = glm::inverse(cameraModelMat);
		return prespective * view;
	}
}