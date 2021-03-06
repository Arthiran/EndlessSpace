#include "FollowPathBehaviour.h"

#include "Timing.h"
#include <Transform.h>

void FollowPathBehaviour::Update(entt::handle entity) {
	if (Points.size() >= 2) {
		Transform& transform = entity.get<Transform>();

		const glm::vec3 next = Points[_nextPointIx];
		const glm::vec3 direction = glm::normalize(next - transform.GetLocalPosition());
		//transform.LookAt(next);
		transform.MoveLocalFixed(direction * Speed * Timing::Instance().DeltaTime);
		if (transform.GetLocalPosition().y > 6.0f)
		{
			transform.SetLocalPosition(transform.GetLocalPosition().x, transform.GetLocalPosition().y - 50.0f, transform.GetLocalPosition().z);
		}
		if (glm::distance(transform.GetLocalPosition(), next) < Speed * Timing::Instance().DeltaTime) {
			_nextPointIx++;
			if (_nextPointIx >= Points.size()) {
				_nextPointIx = 0;
			}
		}
	}
}
