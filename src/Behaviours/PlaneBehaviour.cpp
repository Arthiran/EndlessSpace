#include "PlaneBehaviour.h"

#include "Application.h"
#include "Timing.h"
#include "Transform.h"


void PlaneBehaviour::OnLoad(entt::handle entity) {
	//_initial = entity.get<Transform>().GetLocalRotationQuat();
}

void PlaneBehaviour::Update(entt::handle entity)
{
	float dt = Timing::Instance().DeltaTime;
	GLFWwindow* window = Application::Instance().Window;
	Transform& transform = entity.get<Transform>();

	glm::vec3 movement = glm::vec3(0.0f); 
	//movement.z += -1.0f * dt;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		movement.x += -1.0f * dt;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		movement.x += 1.0f * dt;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		movement.z += -1.0f * dt;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		movement.z += 1.0f * dt;
	}
	transform.MoveLocal(movement * speed);
}
