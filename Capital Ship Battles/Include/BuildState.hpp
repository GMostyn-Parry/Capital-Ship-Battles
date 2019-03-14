/*
 * Author: George Mostyn-Parry
 *
 * A state class for managing, and drawing, the user creating their own ship.
 * Currently only allows turrets to be added.
 */
#pragma once

#include <SFML/Graphics.hpp> //For SFML graphics objects.

#include <vector> //For list of turrets.
#include <memory> //For smart pointers.

#include "AbstractGameState.hpp" //Base state class.
#include "GameManager.hpp" //For changing state, and other high-level information.
#include "ResourceManager.hpp" //So we can load in resources for the state.
#include "Ship.hpp" //So we can show the ship we are building.
#include "Button.hpp" //For GUI buttons.

 //Manages the construction of a ship; allows turrets to be added to the ship.
class BuildState : public AbstractGameState
{
public:
	//Basic BuildState constructor.
	//	game : The state manager, and holder of high-level information on the game.
	BuildState(GameManager &game);

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
private:
	static const sf::Color COLOUR_PLACEABLE; //Colour of unobstructed placeable.
	static const sf::Color COLOUR_OBSTRUCTED; //Colour of obstructed placeable.

	GameManager &m_game; //The game manager; for changing state, and other high-level information.

	Ship m_hull; //The hull of the ship we are building/modifying.
	std::vector<std::unique_ptr<Turret>> m_turretList; //List of all of the turrets attached to the construction.

	ProjectileType m_turretProjType; //The projectile type of the turret we are adding.
	Turret m_buildPreview; //Preview of the turret to be placed.		

	Button m_singleplayerButton; //Buttons that proceeds directly to the battle state.
	Button m_multiplayerButton; //Buttons that proceeds to the connect state.

	Button m_laserButton; //Button that changes the projectile type to laser.
	Button m_missileButton; //Button that changes the projectile type to missile.
	Button m_plasmaButton; //Button that changes the projectile type to plasma.
	sf::Text m_turretTypeText; //Text that displays the current turret type.

	//Changes the projectile type of the turret to be added to the projectile type passed.
	//	projType : The new projectile type of the turret to be added.
	void setProjectileType(ProjectileType projType);

	//Adds a turret to the ship under construction, where the preview is.
	//Returns whether the turret was successfully placed.
	bool addTurret();
	//Removes all turrets that are colliding with the preview.
	void clearArea();

	//Returns the build info of the turrets attached to the ship.
	std::vector<TurretInfo> getTurretBuildInfo() const;

	//Returns whether the placeable can be placed at this location.
	bool isValidPlacement(const sf::RectangleShape &placeable);

	//Places the label denoting the turret type text in the centre-top of the screen.
	void updateTurretTypeText();

	//Switches directly to the battle state.
	void startSingleplayer();
	//Switches to the connect state, so we can attempt to join another user.
	void startMultiplayer();
};