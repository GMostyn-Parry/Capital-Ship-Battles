/*
 * Author: George Mostyn-Parry
 *
 * File for utility functions used by Capital Ship Battles, and constants that go with them.
 */
#pragma once

#include <SFML/Graphics.hpp> //For sf::Transformable, and other SFML classes.

//Constants and functions that are used by the Capital Ship Battles program.
namespace CSB
{
	constexpr float PI = 3.14159265359f; //PI, the mathematical constant; declared as a float as most operations are with floats.
	constexpr unsigned int DEGREES_PER_SECOND = 45; //How many degrees an entity turns per second.

	//Find the angle of the passed vector.
	//	vector : Vector we are finding the angle of.
	//Returns the angle between the points as a positive angle value, as that is what SFML uses.
	inline float vectorAngle(const sf::Vector2f &vector)
	{
		//Converts from radiants, and turns positive.
		return fmod(atan2(vector.y, vector.x) * (180.f / PI) + 360.f, 360.f);
	}

	//Find the angle between the source and the target.
	//	source : The point we are finding the angle from.
	//	target : The point we are finding the angle to.
	//Returns the angle between the points as a positive angle value, as that is what SFML uses.
	inline float angleToPoint(const sf::Vector2f &source, const sf::Vector2f &target)
	{
		return vectorAngle(target - source);
	}

	//Causes the passed entity to turns towards the target.
	//	entity : The entity we are rotating.
	//	target : The point we are rotating to face.
	//	deltaTime : The amount of time that has passed since the last update.
	//Returns whether the entity is now facing the target.
	bool faceTargetAndCheck(sf::Transformable &entity, const sf::Vector2f &target, const sf::Time &deltaTime);
}