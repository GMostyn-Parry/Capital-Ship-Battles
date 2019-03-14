/*
 * Author: George Mostyn-Parry
 *
 * Implementation of projectile class.
 * Possible optimisiation is to multiply by fast inverse square root when calculating velocity.
 */
#include "Projectile.hpp"

#include "CSB_Functions.hpp" //For calculating angle to face the target on spawn.

//Construct a projectile with the passed ShotInfo.
//	info : Information on how to construct the projectile.
Projectile::Projectile(const ShotInfo &info)
	:m_projType(info.projType), m_layer(info.layer)
{
	//How many co-ordinates per second the projectile should move.
	float speed;

	//Customise the projectile depending on what type it is.
	switch(m_projType)
	{
		case ProjectileType::LASER:
			setSize({8, 4});
			setFillColor(sf::Color::Red);
			speed = 1000;
			break;
		case ProjectileType::MISSILE:
			setSize({24, 8});
			setFillColor(sf::Color::Cyan);
			speed = 750;
			break;
		case ProjectileType::PLASMA:
			setSize({10, 10});
			setFillColor(sf::Color::Green);
			speed = 600;
			break;
	}

	setPosition(info.spawn);
	//Centre the origin of the projectile.
	setOrigin(getSize() / 2.f);

	//Distance between the target and the projectile's starting position.
	sf::Vector2f diff = info.target - getPosition();
	setRotation(CSB::vectorAngle(diff));

	//Vector length of the difference.
	float diffLength = sqrt(diff.x * diff.x + diff.y * diff.y);
	//Normalise distance vector and multiply by speed to get the projectile's velocity.
	m_velocity = diff / diffLength * speed;
}

//Processes the projectile for this tick.
//	deltaTime : How much time has passed since the last update.
void Projectile::update(const sf::Time &deltaTime)
{
	move(m_velocity * deltaTime.asSeconds());
}