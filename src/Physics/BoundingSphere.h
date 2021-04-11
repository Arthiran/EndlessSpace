#pragma once
#include <GLM/glm.hpp>
#include "IntersectData.h"

/**
 * The BoundingSphere class represents an sphere that can be used as a
 * collider in a physics engine.
 */
class BoundingSphere
{
public:
	/**
	 * Creates a BoundingSphere in a usable state.
	 *
	 * @param center The center point of the sphere.
	 * @param radius The distance from any point on the sphere to the center.
	 */
	BoundingSphere(const glm::vec3& center, float radius) :
		m_center(center),
		m_radius(radius) {}

	/**
	 * Computes information about if this sphere intersects another aphere.
	 *
	 * @param other The sphere that's being tested for intersection with this
	 *                sphere.
	 */
	IntersectData IntersectBoundingSphere(const BoundingSphere& other) const;

	/** Basic getter for the center */
	inline const glm::vec3& GetCenter() const { return m_center; }
	/** Basic getter for the radius */
	inline float GetRadius()           const { return m_radius; }

	/** Performs a Unit Test of this class */
	static void Test();
private:
	/** The center point of the sphere */
	const glm::vec3 m_center;
	/** The distance from any point on the sphere to the center */
	const float    m_radius;
};
