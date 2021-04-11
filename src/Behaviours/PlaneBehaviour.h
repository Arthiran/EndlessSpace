#pragma once
#include "IBehaviour.h"
#include <GLM/glm.hpp>
#include <GLM/gtc/quaternion.hpp>

class PlaneBehaviour : public IBehaviour
{
public:
	void OnLoad(entt::handle entity) override;
	void Update(entt::handle entity) override;

protected:
	glm::quat _initial;
	float speed = 10.0f;
};
