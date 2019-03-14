/*
 * Author: George Mostyn-Parry
 */
#include "Turret.hpp"

#include "CSB_Functions.hpp" //For rotating the turret to face its target.

 //Construct a complete turret from the passed information.
 //	info : Information defining how the turret should be constructed.
 //	parentTransform : Transform of the turret's parent; used for transforming the turret to global co-ordinates.
 //	atlasTexture : Texture that holds all the different sprites the turret can use.
Turret::Turret(TurretInfo info, const sf::Transform *parentTransform, const sf::Texture *atlasTexture)
	:m_projType(info.projType), m_parentTransform(parentTransform)
{
	setPosition(info.localPosition);
	setSize({32, 32});
	//Centre origin.
	setOrigin(getSize() / 2.f);

	setTexture(atlasTexture);
	//Set the part of the texture to use depending on the projectile type of the turret.
	setTextureRect({std::underlying_type_t<ProjectileType>(m_projType) * 32, 0, 32, 32});

	//Set up the turret depending on what projectile type it fired.
	switch(m_projType)
	{
		case ProjectileType::LASER:
			m_reloadTime = sf::seconds(0.5);
			break;
		case ProjectileType::MISSILE:
			m_reloadTime = sf::seconds(1);
			break;
		case ProjectileType::PLASMA:
			m_reloadTime = sf::seconds(3);
			break;
	}

	//Turret can fire immediately, as the "time since last shot" starts at the reload time.
	m_timeSinceLastShot = m_reloadTime;
}

//Updates the turret's state since the last update.
//	deltaTime : The amount of time that has passed since the turret was last updated.
void Turret::update(sf::Time deltaTime)
{
	m_timeSinceLastShot += deltaTime;

	//Rotate towards the target if the turret is tracking a target, and fire when the turret is facing the target.
	if(m_isTrackingTarget && CSB::faceTargetAndCheck(*this, m_parentTransform->getInverse().transformPoint(m_targetPosition), deltaTime))
	{
		fire();
	}
}

//Orders the turret to fire at the specified target.
//	target : Where the turret should fire at.
//	layer : Which layer the shot should be fired onto.
void Turret::fireCommand(const sf::Vector2f & target, unsigned int layer)
{
	//Queue order to fire if the turret has not fired too recently.
	if(m_timeSinceLastShot > m_reloadTime)
	{
		m_isTrackingTarget = true;
		m_targetPosition = target;
		m_targetlayer = layer;
	}
}

//Pushes information on a projectile to be created onto the firing list.
void Turret::fire()
{
	s_fireList->push_back({m_projType, m_targetlayer, m_parentTransform->transformPoint(getPosition()), m_targetPosition});

	m_isTrackingTarget = false;
	m_timeSinceLastShot = sf::seconds(0);
}