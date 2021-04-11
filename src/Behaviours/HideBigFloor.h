#pragma once
#include "IBehaviour.h"
#include <vector>
#include <GLM/glm.hpp>

class HideBigFloor final : public IBehaviour
{
public:

	void Update(entt::handle entity) override;
	
};