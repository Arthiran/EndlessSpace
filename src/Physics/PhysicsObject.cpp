#include "PhysicsObject.h"

#include "Application.h"
#include "Timing.h"
#include "Transform.h"

void PhysicsObject::Update(entt::handle entity)
{
	float dt = Timing::Instance().DeltaTime;
	GLFWwindow* window = Application::Instance().Window;
	Transform& transform = entity.get<Transform>();

	m_position = transform.GetLocalPosition() + m_offset;
	//printf("Distance: %f \n", transform.GetLocalPosition().y);
}

void PhysicsObject::SetPositionAndRadius(glm::vec3 position, glm::vec3 offset, float radius)
{
	m_position = position;
	m_offset = offset;
	m_radius = radius;
}

void PhysicsObject::SetIsPlayer(bool isPlayer)
{
	m_isPlayer = isPlayer;
}

glm::vec3 PhysicsObject::GetPosition()
{
	return m_position;
}

float PhysicsObject::GetRadius()
{
	return m_radius;
}

bool PhysicsObject::GetIsPlayer()
{
	return m_isPlayer;
}
