/*
 * Author: George Mostyn-Parry
 *
 * Abstract class to represent a generic state the game can be in; i.e. the build state, where the player constructs their ship.
 */
#pragma once

#include <SFML/Graphics.hpp> //For SFML graphics, and other features.

//Abstract class for any main state the game may be in.
class AbstractGameState : public sf::Drawable
{
public:
	//Passes events to the state, so the state may handle them itself.
	//	event : The event for the state to handle.
	virtual void handleInput(const sf::Event &event) = 0;
	//Processes the state logic to move the state forward a tick.
	//	deltaTime : The amount of time that has passed since the last update.
	virtual void update(const sf::Time &deltaTime) = 0;

	//Update the state's view, i.e. fix the GUI, and other elements, from a window resize.
	virtual void updateView() = 0;
};

