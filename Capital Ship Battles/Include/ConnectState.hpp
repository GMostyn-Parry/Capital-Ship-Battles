/*
 * Author: George Mostyn-Parry
 *
 * State class to represent connecting to a peer; used for both hosts, and clients.
 */
#pragma once

#include <vector> //For vectors.

#include "AbstractGameState.hpp" //Base class.
#include "GameManager.hpp" //For changing state, and referring to the render window.
#include "Button.hpp" //For GUI buttons.

//State class to represent connecting to a peer; used for both hosts, and clients.
class ConnectState : public AbstractGameState
{
public:
	//Basic ConnectState constructor.
	//	game : The state manager, and holder of high-level information on the game.
	ConnectState(GameManager &game);
	//ConnectState destructor.
	~ConnectState();

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
	GameManager &m_game; //The manager of the current state, and high-level game information.

	Button m_leaveButton; //Button that takes us back to the build state.
	std::vector<Button> m_buttonList; //List of all the buttons currently being used by the state.
	std::vector<sf::Text> m_labelList; //List of all the labels currently being used by the state.

	bool m_isJoining = false; //Whether we need to track text input for the joining state.
	bool m_hasPeer = false; //Whether we are connected to another player.

	std::unique_ptr<sf::Thread> m_connectThread; //Thread for connecting to another player.	

	//Changes to the hosting state of the connect state.
	void enterHostState();
	//Changes to the joining state of the connect state.
	void enterJoinState();

	//Waits for the server to receive a client before switching to the battle state.
	void waitForClient();
	//Attempt to join a server with the currently entered string input.
	void attemptJoin();

	//Changes the current state to the build state.
	void startBuild();
	//Changes the current state to a networked battle.
	void startBattle();
};

