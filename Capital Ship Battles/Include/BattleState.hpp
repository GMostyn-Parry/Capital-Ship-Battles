/*
 * Author: George Mostyn-Parry
 *
 * Game state for managing battles; the main game state.
 * Allows for ships, and projectiles to be created; handles collisions, and processes the battle each tick.
 */
#pragma once

#include <memory> //For smart pointers.

#include "AbstractGameState.hpp" //Base class.
#include "GameManager.hpp" //For high-level information, and state changing.
#include "Ship.hpp" //For the ships that fight in the battle state.

//Manages a battle; adds ships, creates and resolves projectiles, and processes each game tick.
class BattleState : public AbstractGameState
{
public:
	//Basic BattleState constructor.
	//	game : The state manager, and holder of high-level information on the game.
	//	isMultiplayer : Whether this battle is being networked.
	BattleState(GameManager &game, bool isMultiplayer = false);
	//BattleState destructor.
	~BattleState();

	//Passes events to the state, so the state may handle them itself.
	//	event : The event for the state to handle.
	virtual void handleInput(const sf::Event &event);
	//Processes the state logic to move the state forward a tick.
	//	deltaTime : The amount of time that has passed since the last update.
	virtual void update(const sf::Time &deltaTime);
	//Draws all renderable elements of the state to the screen.
	//	target : What we will be drawing onto.
	//	states : Visual manipulations to the elements that are being drawn.
	virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const;

	//Update the state's view, i.e. fix the GUI, and other elements, from a window resize.
	virtual void updateView();

	//Creates a ship with the passed information.
	//	team : The team the ship belongs to.
	//	position : Where the ship should start on creation.
	//	angle : The rotation of the ship on creation.
	//	turretBuildList : The turrets the ship should start with.
	void createShip(unsigned int team, const sf::Vector2f &position, float angle, const std::vector<TurretInfo> &turretBuildList = std::vector<TurretInfo>());

	//Passes a move command to the specified ship.
	//	shipLayer : The layer the ship is on; i.e. which team.
	//	shipID : The ID of the ship in the team.
	//	destination : Where the ship is being told to head to.
	void issueMoveCommand(unsigned int shipLayer, unsigned int shipID, const sf::Vector2f &destination);
	//Passes a fire command to the specified ship.
	//	shipLayer : The layer the ship is on; i.e. which team.
	//	shipID : The ID of the ship in the team.
	//	target : Where the ship is being told to shoot at.
	//	targetLayer : Which layer the ship is being told to shoot on.
	void issueFireCommand(unsigned int shipLayer, unsigned int shipID, const sf::Vector2f &target, unsigned int targetLayer);
private:
	GameManager &m_game; //The game manager; for changing state, and other high-level information.

	bool m_isFinished = false; //Whether the battle is finished, and ready to head to the next state.

	std::vector<std::unique_ptr<Projectile>> m_projList; //List of all active projectiles.
	std::vector<std::unique_ptr<Ship>>m_shipList[2]; //List of all active ships; the array is a reference to the layer the ship is located on.
	std::vector<ShotInfo> m_readyToFire; //List of shots ready to be fired/created.

	sf::Thread m_networkThread; //Thread responsible for networking.

	sf::View m_gameView; //The view the battle is drawn to.
	sf::FloatRect m_viewBounds; //Where the battle's view should constrain itself to.
	
	sf::RectangleShape areaBorder; //Visual representation of the view bounds.
	
	//Creates a projectile with the passed information.
	//	info : The information used to create the projectile.
	void createProjectile(const ShotInfo &info);

	//Processes a single tick for all projectiles.
	//	deltaTime : The amount of time that has passed since the last update.
	void resolveProjectiles(const sf::Time &deltaTime);
	//Determines if the projectile collided with anything.
	//	proj : The projectile we are checking collisions for.
	//	deltaTime : The amount of time that has passed since the last update.
	//Returns whether a collision occurred.
	bool collide(const std::unique_ptr<Projectile> &proj, const sf::Time &deltaTime);

	//Ends the battle state, and proceeds to the build state.
	void changeToBuildState();
};