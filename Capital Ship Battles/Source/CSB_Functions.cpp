/*
 * Author: George Mostyn-Parry
 */
#include "CSB_Functions.hpp"

namespace CSB
{
	//Causes the passed entity to turns towards the target.
	//	entity : The entity we are rotating.
	//	target : The point we are rotating to face.
	//	deltaTime : The amount of time that has passed since the last update.
	//Returns whether the entity is now facing the target.
	bool faceTargetAndCheck(sf::Transformable &entity, const sf::Vector2f &target, const sf::Time &deltaTime)
	{
		//Angle between the entity, and the target.
		float angle = angleToPoint(entity.getPosition(), target);
		//Difference between the target angle and the current angle.
		float angleDifference = angle - entity.getRotation();
		//Direction we are turning.
		int direction = 1;

		//Invert the direction if the angle difference is negative and less than 180,
		//or the difference is greater than 180 and the target angle is larger than the current rotation.
		if((angleDifference < 0 && abs(angleDifference) < 180) ||
			(abs(angleDifference) >= 180 && angle > entity.getRotation()))
		{
			direction = -1;
		}

		//How much the entity should rotate this tick.
		float turn = direction * deltaTime.asSeconds() * DEGREES_PER_SECOND;
		//Set the rotation to the target, and return true, if the turn will bring us to the target.
		if(abs(angleDifference) <= abs(turn))
		{
			entity.rotate(angleDifference);
			return true;
		}
		//Otherwise, simply turn toward the target and return false.
		else
		{
			entity.rotate(turn);
			return false;
		}
	}
}