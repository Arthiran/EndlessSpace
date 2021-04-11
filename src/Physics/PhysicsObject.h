#pragma once
#include "IBehaviour.h"
#include <GLM/glm.hpp>
#include <GLM/gtc/quaternion.hpp>
#include "BoundingSphere.h"

class PhysicsObject : public IBehaviour
{
public:
	void Update(entt::handle entity) override;

	/** Basic getter */
	inline const glm::vec3& GetPosition() const { return m_position; }
	/** Basic getter */
	inline float GetRadius() const { return m_radius; }

	/**
	 * Returns a bounding sphere around this object;
	 * TODO: This is termporary!
	 */
	inline BoundingSphere GetBoundingSphere() const
	{
		return BoundingSphere(m_position, m_radius);
	}

	void SetPositionAndRadius(glm::vec3 position, glm::vec3 offset, float radius);
	void SetIsPlayer(bool isPlayer);

	glm::vec3 GetPosition();
	float GetRadius();
	bool GetIsPlayer();

private:
	/** Where this object is in 3D space. */
	glm::vec3 m_position;
	glm::vec3 m_offset;
	/** The radius of this object; TODO: this is temporary! */
	float	m_radius;
	bool	m_isPlayer = false;
};
