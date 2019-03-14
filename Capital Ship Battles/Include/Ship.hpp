/*
 * Author: George Mostyn-Parry
 *
 * A class that represents a ship in the game, and handles the updating of the internal state each update tick.
 * Has a list of turrets, which it defers firing actions to.
 * Takes damage by hiding pixels that have been marked as hit on a destruction key texture, which is fed to a fragment shader.
 * Employs Bresenham's line algorithm to determine which pixel was struck on the ship.
 *
 * Does not have movement collision; i.e. it will move through any obstacle.
 */
#pragma once

#include <vector> //For vector lists.
#include <memory> //For smart pointers.

#include "Turret.hpp" //For turrets mounted on the ship.

//A moving ship in the game that can fire its turrets, and taken per-pixel damage on a projectile collision.
class Ship : public sf::Sprite
{
public:
	//Construct ship with passed parameters.
	//	position : Position the ship starts at.
	//	angle : Angle the ship starts at.
	//	turretList : List of turrets the ship starts with.
	//	hullTexture : Texture of the ship hull.
	//	turretAtlasTexture : Texture atlas for the turrets.
	Ship(const sf::Vector2f &position, float angle, const std::vector<TurretInfo> &turretList, const sf::Texture *hullTexture, const sf::Texture *turretAtlasTexture);
	
	//Causes the ship to process internal data to update its state for this tick.
	//	deltaTime : The amount of time that has passed since the last update.
	void update(const sf::Time &deltaTime);
	//Draw the ship, and its children, to the render target.
	//	target : What we will draw the ship onto.
	//	states : How to manipulate the drawing of the ship.
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	//Orders the ship to move to the target position.
	//	target : The position to move to.
	void moveCommand(const sf::Vector2f &target);
	//Orders the ship's turrets to fire at the target position.
	//	target : The position to fire at.
	//	layer : The layer the shot will travel on.
	void fireCommand(const sf::Vector2f &target, unsigned int layer);

	//Finds if there was a collision between this ship and the passed global position.
	//	globalPosition : The global position to check for a collision against.
	//Returns whether the collision occurred.
	bool collide(const sf::Vector2f &globalPosition);
	//Finds if there was a collision between this ship and the passed projectile.
	//	proj : Projectile that might have collided with this ship.
	//	deltaTime : The amount of time that passed to move the projectile to this position from the last.
	//Returns whether the collision occurred.
	bool collide(const Projectile &proj, const sf::Time &deltaTime);

	//Builds, and adds, turrets made from the build info to this ship.
	//	newTurrets : Build information for the new turrets.
	//	turretAtlasTexture : Texture atlas to apply to the new turrets.
	void addTurrets(const std::vector<TurretInfo> &newTurrets, const sf::Texture *turretAtlasTexture);

	//Returns whether the ship has finished all processing, and needs to be cleaned up by the game.
	bool requiresCleanup() const;
private:
	//The different movement states a ship may be in.
	enum class MovementState
	{
		IDLE, ROTATING, MOVING, DECELERATING
	};

	static constexpr unsigned int KEY_SIZE_FACTOR = 4; //The factor the destruction key is smaller than the actual texture.

	MovementState m_movementState = MovementState::IDLE; //The movement state the ship is currently in.
	float m_speed = 0; //How many global co-ordinates the ship will move per second.
	float m_acceleration = 20; //How much the velocity will increase per second when accelerating.
	float m_deceleration = 10; //How much the velocity will decrease per second when decelerating.
	sf::Vector2f m_destination; //Where the ship is currently travelling to.
	
	std::vector<std::unique_ptr<Turret>> m_turrets; //List of turrets attached to this ship.

	sf::Image m_keyImage; //The image representing the key of whether a hull pixel is destroyed.
	sf::Texture m_keyTex; //The texture that holds the information of the key's image.
	sf::Shader m_damageShader; //Shader that uses the damage key to differentiate between which pixels should be visible.

	//Converts global co-ordinates to the pixel this corresponds to relative to the ship's texture.
	//	globalPosition : The global co-ordinates to to transform.
	//Returns the pixel co-ordinates that the global co-ordinates transformed to.
	sf::Vector2i getPixelPosition(sf::Vector2f globalPosition) const;
	//Finds the first pixel hit by the projectile.
	//	proj : The projectile that might have collided with the ship.
	//	deltaTime : The amount of time that passed to move the projectile to this position from the last.
	//Returns the first pixel hit, or a value of {-1, -1} if there was no collision.
	sf::Vector2i firstPixelHit(const Projectile &proj, const sf::Time &deltaTime) const;
};

//Returns whether the ship has finished all processing, and needs to be cleaned up by the game.
inline bool Ship::requiresCleanup() const
{
	//The ship is "dead" when it loses all turrets.
	return m_turrets.size() == 0;
}