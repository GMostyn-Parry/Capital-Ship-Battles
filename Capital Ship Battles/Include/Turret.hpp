/*
 * Author: George Mostyn-Parry
 *
 * A turret that fires projectiles at a target.
 * The turret will turn to face the target before queueing the shot onto a fire list,
 * which should be used to create the projectiles in a way that it appears as though the turret fired the projectile.
 */
#pragma once

#include "Projectile.hpp" //The projectile the turret will shoot.

 //The information local to the turret to allows it to be constructed.
//Lightweight memory usage compared to storing compies of the turret class.
struct TurretInfo
{
	ProjectileType projType; //The type of projectile the turret shoots.
	sf::Vector2f localPosition; //Turret's position relative to the parent.
};

//Turret that turns to face its target before firing a projectile.
class Turret : public sf::RectangleShape
{
public:
	static std::vector<ShotInfo> *s_fireList; //The list of shots that are to be used to create projectiles.

	//Construct a complete turret from the passed information.
	//	info : Information defining how the turret should be constructed.
	//	parentTransform : Transform of the turret's parent; used for transforming the turret to global co-ordinates.
	//	atlasTexture : Texture that holds all the different sprites the turret can use.
	Turret(TurretInfo info, const sf::Transform *parentTransform, const sf::Texture *atlasTexture);

	//Updates the turret's state since the last update.
	//	deltaTime : The amount of time that has passed since the turret was last updated.
	void update(sf::Time deltaTime);

	//Returns information on how to construct this turret with the TurretInfo struct.
	TurretInfo getTurretInfo() const;

	//Orders the turret to fire at the specified target.
	//	target : Where the turret should fire at.
	//	layer : Which layer the shot should be fired onto.
	void fireCommand(const sf::Vector2f &target, unsigned int layer);
private:
	const sf::Transform *m_parentTransform; //The transform that the turret is parented to.

	ProjectileType m_projType; //The type of projectile the turret fires.
	
	sf::Time m_reloadTime; //How long the turret must wait between shots.
	sf::Time m_timeSinceLastShot; //How long since the turret last fired.

	bool m_isTrackingTarget = false; //Whether the turret is tracking the target position to fire at it.
	sf::Vector2f m_targetPosition; //Where the turret is firing at.
	unsigned int m_targetlayer; //What layer the turret should fire on to.

	//Pushes information on a projectile to be created onto the firing list.
	void fire();
};

//Returns information on how to construct this turret with the TurretInfo struct.
inline TurretInfo Turret::getTurretInfo() const
{
	return {m_projType, getPosition()};
}