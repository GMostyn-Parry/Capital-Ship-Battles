/*
 * Author: George Mostyn-Parry
 *
 * A class and data types for creating and updating a projectile.
 */
#pragma once

#include <SFML/Graphics.hpp> //For SFML graphics objects.

//All of the different projectile types.
enum class ProjectileType : uint8_t
{
	LASER,
	MISSILE,
	PLASMA
};

//Information on a firing action that is to spawn a projectile.
struct ShotInfo
{
	ProjectileType projType; //The projectile's type.
	unsigned int layer; //Which "layer" the projectile is on; i.e. which team it should hit.
	sf::Vector2f spawn; //Where the projectile starts from.
	sf::Vector2f target; //Where the projectile is heading towards from its spawn position.
};

//Class on projectiles that will travel across the screen to hit a target.
class Projectile : public sf::RectangleShape
{
public:
	//Construct a projectile with the passed ShotInfo.
	//	info : Information on how to construct the projectile.
	Projectile(const ShotInfo &info);
	//Processes the projectile for this tick.
	//	deltaTime : How much time has passed since the last update.
	void update(const sf::Time &deltaTime);

	//Returns the projectile's type.
	ProjectileType getProjectileType() const;
	//Returns the layer the projectile exists on.
	unsigned int getLayer() const;
	//Returns the velocity the projectile is travelling at.
	const sf::Vector2f& getVelocity() const;

	//Returns whether the projectile has been marked for clean-up.
	bool requiresCleanup() const;
private:
	ProjectileType m_projType; //The projectile's type.
	unsigned int m_layer; //Which layer the projectile is on; i.e. which team it should hit.
	sf::Vector2f m_velocity; //Velocity of the projectile.
	bool m_isFinished = false; //Whether the projectile is finished, and needs cleaning up.
};

//Returns the projectile's type.
inline ProjectileType Projectile::getProjectileType() const
{
	return m_projType;
}

//Returns the layer the projectile exists on.
inline unsigned int Projectile::getLayer() const
{
	return m_layer;
}

//Returns the velocity the projectile is travelling at.
inline const sf::Vector2f& Projectile::getVelocity() const
{
	return m_velocity;
}

//Returns whether the projectile has been marked for clean-up.
inline bool Projectile::requiresCleanup() const
{
	return m_isFinished;
}