#include "Window.hpp"

#include "../DaxaCore.hpp"

#include <iostream>

#include "../gpu/Instance.hpp"

namespace daxa {

	void closeCallback(GLFWwindow* window) {
		WindowState* self = reinterpret_cast<WindowState*>(glfwGetWindowUserPointer(window));
		self->bCloseRequested = true;
	}

	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		WindowState* self = reinterpret_cast<WindowState*>(glfwGetWindowUserPointer(window));
		if (action == GLFW_PRESS) {
			self->KeyDown[key] = true;
		} else if (action == GLFW_RELEASE) {
			self->KeyDown[key] = false;
		}
	}

	void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		WindowState* self = reinterpret_cast<WindowState*>(glfwGetWindowUserPointer(window));
		if (action == GLFW_PRESS) {
			self->mouseButtonDown[button] = true;
		} else if (action == GLFW_RELEASE) {
			self->mouseButtonDown[button] = false;
		}
	}

	void cursorMoveCallback(GLFWwindow* window, double xpos, double ypos) {
		WindowState* self = reinterpret_cast<WindowState*>(glfwGetWindowUserPointer(window));
		self->cursorChangeX = static_cast<i32>(std::floor(xpos)) - self->oldCursorPosX;
		self->cursorChangeY = static_cast<i32>(std::floor(ypos)) - self->oldCursorPosY;
	}

	Window::Window(std::string name, u32 width, u32 height) 
		: name{ name }
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		if (glfwWindow = glfwCreateWindow(width, height,"glfw3 window", nullptr, nullptr); !glfwWindow) {
			printf("error could not create window!\n");
		}
		glfwSetWindowUserPointer(glfwWindow, windowState.get());

		VkResult err = glfwCreateWindowSurface(instance->getVkInstance(), glfwWindow, nullptr, &surface);
		DAXA_ASSERT_M(err == VK_SUCCESS, "could not create surface for window");
		glfwSetWindowCloseCallback(glfwWindow, closeCallback);
		glfwSetKeyCallback(glfwWindow, KeyCallback);
		glfwSetMouseButtonCallback(glfwWindow, mouseButtonCallback);
		glfwSetCursorPosCallback(glfwWindow, cursorMoveCallback);
	}

	Window::~Window() 	{
		vkDestroySurfaceKHR(instance->getVkInstance(), surface, nullptr);
		glfwDestroyWindow(glfwWindow);
		glfwWindow = nullptr;
		surface = nullptr;
	}

	const std::string& Window::getName() {
		return name;
	}

	bool Window::isFocused() const {
		return bFocused;
	}

	// keys

	bool Window::keyPressed(Key key) const {
		return windowState->KeyDown[key];
	}

	bool Window::keyJustPressed(Key key) const {
		return !windowState->KeyDownOld[key] && windowState->KeyDown[key];
	}

	bool Window::keyJustReleased(Key key) const {
		return windowState->KeyDownOld[key] && !windowState->KeyDown[key];
	}

	// buttons

	bool Window::buttonPressed(Button button) const {
		return windowState->mouseButtonDown[button];
	}

	bool Window::buttonJustPressed(Button button) const {
		return !windowState->mouseButtonDownOld[button] && windowState->mouseButtonDown[button];
	}

	bool Window::buttonJustReleased(Button button) const {
		return windowState->mouseButtonDownOld[button] && !windowState->mouseButtonDown[button];
	}

	// cursor

	i32 Window::getCursorPosX() const{
		double x,y;
		glfwGetCursorPos(glfwWindow, &x,&y);
		return static_cast<i32>(std::floor(x));
	}

	i32 Window::getCursorPosY() const {
		double x,y;
		glfwGetCursorPos(glfwWindow, &x,&y);
		return static_cast<i32>(std::floor(y));
	}

	i32 Window::getCursorPosChangeX() const {
		return windowState->cursorChangeX;
	}

	i32 Window::getCursorPosChangeY() const {
		return windowState->cursorChangeY;
	}

	bool Window::isCursorOverWindow() const {
		double x,y;
		glfwGetCursorPos(glfwWindow, &x,&y);
		i32 width, height;
		glfwGetWindowSize(glfwWindow, &width, &height);
		return x >= 0 && x <= width && y >= 0 && y <= height;
	}

	void Window::captureCursor() {
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void Window::releaseCursor() {
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	bool Window::isCursorCaptured() const {
		return glfwGetInputMode(glfwWindow, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
	}

	bool Window::update(f32 deltaTime) 	{
		windowState->KeyDownOld = windowState->KeyDown;
		windowState->mouseButtonDownOld = windowState->mouseButtonDown;
		windowState->oldCursorPosX = getCursorPosX();
		windowState->oldCursorPosY = getCursorPosY();
		windowState->cursorChangeX = {};
		windowState->cursorChangeY = {};
		glfwPollEvents();
		if (isCursorCaptured()) {
			glfwSetCursorPos(glfwWindow, -10000, -10000);
		}
		return windowState->bCloseRequested;
	}

	GLFWwindow* Window::getGLFWWindow() {
		return glfwWindow;
	}

	VkSurfaceKHR_T* Window::getSurface() {
		return surface;
	}

	void Window::setWidth(u32 width) {
		i32 oldW, oldH;
		glfwGetWindowSize(glfwWindow, &oldW, &oldH);
		glfwSetWindowSize(glfwWindow, width, oldH);
	}

	void Window::setHeight(u32 height) {
		i32 oldW, oldH;
		glfwGetWindowSize(glfwWindow, &oldW, &oldH);
		glfwSetWindowSize(glfwWindow, oldW, height);
	}

	u32 Window::getWidth() const {
		i32 w, h;
		glfwGetWindowSize(glfwWindow, &w, &h);
		return w;
	}

	u32 Window::getHeight() const {
		i32 w, h;
		glfwGetWindowSize(glfwWindow, &w, &h);
		return h;
	}
}