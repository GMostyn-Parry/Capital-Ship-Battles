/*
 * Author: George Mostyn-Parry
 *
 * Manager class for the handling of states, and execution of the game. 
 */
#pragma once

#include <SFML/Graphics.hpp> //For RenderWindow, and other SFML classes and functions.

#include "AbstractGameState.hpp" //For changing, and processing, game states.
#include "ResourceManager.hpp" //For controlling life-time of resources.
#include "NetworkManager.hpp" //For networking battles between two users.

//State and execution manager for the game.
class GameManager
{
public:
	std::vector<TurretInfo> turretBuildList; //List of information to build the turret configuration the local player made.

	//Default GameManager constructor.
	GameManager();

	//The loop that handles the execution of the game.
	void gameLoop();

	//Sets the state currently being run by the state manager; the old state is destroyed.
	//	newState : The state that will become the current state.
	void setState(std::unique_ptr<AbstractGameState> newState);

	//Returns a reference to the resource manager.
	ResourceManager& getResourceManager();
	//Returns a reference to the network manager.
	NetworkManager& getNetworkManager();
	//Returns a const reference to the window we are drawing to.
	const sf::RenderWindow& getWindow() const;
private:
	const sf::Time FIXED_UPDATES_PER_SECOND = sf::seconds(1.f / 60.f); //How fast to update the game/physics process.

	bool m_isWaitingForRenderingEnd = false; //Whether the main thread is waiting for the rendering thread to end.

	std::unique_ptr<AbstractGameState> m_currentGameState; //The state the game is currently in.
	ResourceManager m_resourceManager; //The object that manages the resources used by the game.
	NetworkManager m_networkManager; //Manager for networking battles between two users.

	sf::RenderWindow m_window; //Window we will be rendering to.
	sf::View staticView = sf::View(m_window.getView()); //Static view for GUI elements; i.e. the window's view.

	sf::Thread m_renderThread; //Thread responsible for rendering.
	sf::Mutex m_stateChangeMutex; //Controls access to states while the state is being changed.

	//Updates view on the window's size being changed.
	void updateView();

	//Draw the state elements to the window.
	//Placed on a seperate thread as we don't want the amount of draw calls to slow down the main thread.
	void draw();
	//Safely starts drawing on the rendering thread.
	void startRenderThread();
	//Causes the rendering thread to safely close; useful for safely closing, or recreating the window.
	void endRenderThread();
};

//Returns a reference to the resource manager.
inline ResourceManager& GameManager::getResourceManager()
{
	return m_resourceManager;
}

//Returns a reference to the network manager.
inline NetworkManager& GameManager::getNetworkManager()
{
	return m_networkManager;
}

//Returns a const reference to the window we are drawing to.
inline const sf::RenderWindow& GameManager::getWindow() const
{
	return m_window;
}