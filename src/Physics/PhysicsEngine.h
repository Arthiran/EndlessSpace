#pragma once
#include "IBehaviour.h"
#include <GLM/glm.hpp>
#include <GLM/gtc/quaternion.hpp>
#include "PhysicsObject.h"
#include <vector>
#include "Scene.h"
#include <Utilities/EffectHandler.h>

/**
 * The PhysicsEngine encapsulates all the functions and information necessary
 * to perform a physics simulation.
 */
class PhysicsEngine : public IBehaviour
{
public:
	void OnLoad(entt::handle entity) override;
	void Update(entt::handle entity) override;

	void AddObject(std::shared_ptr<PhysicsObject> object);

	void HandleCollisions();

	//TODO: Temporary Getters
	std::shared_ptr<PhysicsObject> GetObject(unsigned int index);
	unsigned int GetNumObjects();

	void SetEffectHandler(EffectHandler* effectHandler);

private:
	/** All the objects being simulated by the PhysicsEngine. */
	std::vector<std::shared_ptr<PhysicsObject>> m_objects;
	EffectHandler* m_effectHandler;
};
