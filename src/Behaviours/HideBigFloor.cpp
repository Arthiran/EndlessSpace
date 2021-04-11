#include "HideBigFloor.h"

#include "Application.h"
#include "Timing.h"
#include <Transform.h>

void HideBigFloor::Update(entt::handle entity) {
	Transform& transform = entity.get<Transform>();
	GLFWwindow* window = Application::Instance().Window;

	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
		transform.SetLocalPosition(transform.GetLocalPosition().x, transform.GetLocalPosition().y, transform.GetLocalPosition().z - 1000);
	}
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
		transform.SetLocalPosition(transform.GetLocalPosition().x, transform.GetLocalPosition().y, 0);
	}
}
