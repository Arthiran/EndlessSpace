#pragma once

class IntersectData
{
public:
	/**
	 * Creates Intersect Data in a usable state.
	 *
	 * @param doesIntersect Whether or not the objects are intersecting.
	 * @param distance      The distance between the two objects
	 */
	IntersectData(const bool doesIntersect, const float distance) :
		m_doesIntersect(doesIntersect),
		m_distance(distance) {}

	/** Basic getter for m_doesIntersect */
	inline bool GetDoesIntersect() const { return m_doesIntersect; }
	/** Basic getter for m_distance */
	inline float GetDistance()     const { return m_distance; }
private:
	/** Whether or not the objects are intersecting */
	const bool  m_doesIntersect;
	/** The distance between the two objects */
	const float m_distance;
};
