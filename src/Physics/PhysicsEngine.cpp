#include "PhysicsEngine.h"

#include "Application.h"
#include "Timing.h"
#include "Transform.h"
#include "BoundingSphere.h"

void PhysicsEngine::OnLoad(entt::handle entity) {
	//_initial = entity.get<Transform>().GetLocalRotationQuat();
}

void PhysicsEngine::Update(entt::handle entity)
{
	float dt = Timing::Instance().DeltaTime;
	GLFWwindow* window = Application::Instance().Window;
	Transform& transform = entity.get<Transform>();

	//printf("Position: %f \n", m_objects[1]->GetPosition().y);
	HandleCollisions();
}

void PhysicsEngine::AddObject(std::shared_ptr<PhysicsObject> object)
{
	m_objects.push_back(object);
}


void PhysicsEngine::HandleCollisions()
{
	for (unsigned int i = 0; i < m_objects.size(); i++)
	{
		if (m_objects[i]->GetIsPlayer())
		{
			for (unsigned int j = i + 1; j < m_objects.size(); j++)
			{
				glm::vec3 FirstObject = glm::vec3(m_objects[i]->GetPosition().x, m_objects[i]->GetPosition().y, 0);
				glm::vec3 SecondObject = glm::vec3(m_objects[j]->GetPosition().x, m_objects[j]->GetPosition().y, 0);

				float centerDistance = glm::length(SecondObject - FirstObject);
				if (centerDistance < m_objects[i]->GetRadius() + m_objects[j]->GetRadius())
				{
					m_effectHandler->m_activeEffect = 2;
					printf("I am colliding \n");
				}
			}
		}
	}
}

std::shared_ptr<PhysicsObject> PhysicsEngine::GetObject(unsigned int index)
{
	return m_objects[index];
}

unsigned int PhysicsEngine::GetNumObjects()
{
	return (unsigned int)m_objects.size();
}

void PhysicsEngine::SetEffectHandler(EffectHandler* effectHandler)
{
	m_effectHandler = effectHandler;
}
