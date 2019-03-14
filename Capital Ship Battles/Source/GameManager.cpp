/*
 * Author: George Mostyn-Parry
 */
#include "GameManager.hpp"

#include "ResourceManager.hpp" //For universal storage of textures and fonts.

//Default GameManager constructor.
GameManager::GameManager()
	:m_renderThread(&GameManager::draw, this)
{
	//Create window we will be rendering to.
	m_window.create(sf::VideoMode(800, 600), "Capital Ship Battle");
}

//The loop that handles the execution of the game.
void GameManager::gameLoop()
{
	//The manager for the resources the game uses.
	ResourceManager resourceManager; 

	//Safely start rendering thread.
	startRenderThread();

	//The amount of time accumulated since the last game tick.
	sf::Time timeSinceLastTick = FIXED_UPDATES_PER_SECOND;
	//Keeps track of how much time has passed in the cycle.
	sf::Clock gameClock;

	//Keep the game running for as long as the window is open.
	while(m_window.isOpen())
	{
		//Stores the most recently received event.
		sf::Event event;
		//Handle all of the events that happened since the last cycle.
		while(m_window.pollEvent(event))
		{
			//Close the window if the user sends a causes a close event.
			if(event.type == sf::Event::Closed)
			{
				//Wait for execution of the draw thread to end before closing the game.
				endRenderThread();
				m_window.close();
			}
			//Handle any keys pressed on the keyboard.
			else if(event.type == sf::Event::KeyPressed)
			{
				//Handle each key code.
				switch(event.key.code)
				{
					//Close the window, when the escape key is pressed.
					case sf::Keyboard::Escape:
						//Wait for execution of the draw thread to end before closing the game.
						endRenderThread();						
						m_window.close();

						break;
					case sf::Keyboard::Return:
						//Toggle fullscreen when alt+enter is pressed.
						if(event.key.alt)
						{
							//End the draw thread, so we can safely recreate the window.
							endRenderThread();

							
							//Change to the default size {800, 600} when the window is fullscreen.
							//Easiest way to check for this is if the height of the window is the same as the screen height.
							if(m_window.getSize().y == sf::VideoMode::getDesktopMode().height)
							{
								m_window.create(sf::VideoMode(800, 600), "Capital Ship Battle");
							}
							//Otherwise, change to fullscreen.
							else
							{
								m_window.create(sf::VideoMode::getDesktopMode(), "Capital Ship Battle", sf::Style::Fullscreen);
							}

							//Update the game view to match new window.
							updateView();
							
							//We don't want the rendering thread to close itself when we launch it.
							m_isWaitingForRenderingEnd = false;

							//Safely restart rendering thread.
							startRenderThread();
						}
					default:
						break;
				}
			}
			//Adjust the view, so elements are not distorted when the window is resized.
			else if(event.type == sf::Event::Resized)
			{
				updateView();
			}

			//Pass the event to the current state.
			m_currentGameState->handleInput(event);
		}

		//Add the amount of time that occurred since the last tick.
		timeSinceLastTick += gameClock.restart();

		//Update the battle if more time has passed than the update rate per second.
		while(timeSinceLastTick >= FIXED_UPDATES_PER_SECOND)
		{
			m_currentGameState->update(FIXED_UPDATES_PER_SECOND);
			timeSinceLastTick -= FIXED_UPDATES_PER_SECOND;
		}
	}
}

//Sets the state currently being run by the state manager; the old state is destroyed.
//	newState : The state that will become the current state.
void GameManager::setState(std::unique_ptr<AbstractGameState> newState)
{
	//Allows state to be changed without read violation from a draw operation.
	m_stateChangeMutex.lock();

	m_currentGameState = std::move(newState);
	//Update the view of the new state.
	//This is done here, rather than the constructor of the state, as the window might not be using the correct view
	//when the state is constructed; i.e. due to view swapping for GUI vs. game elements.
	m_currentGameState->updateView();

	m_stateChangeMutex.unlock();
}

//Updates view on the window's size being changed.
void GameManager::updateView()
{
	//Prevent distortion by matching the static view to the window's size.
	staticView.setSize(sf::Vector2f(m_window.getSize()));
	m_window.setView(staticView);

	//Fix the GUI from the window size change.
	//This can occur from the window being recreated, which generates no event, so the state manager handles this.
	m_currentGameState->updateView();
}

//Draw the state elements to the window.
//Placed on a seperate thread as we don't want the amount of draw calls to slow down the main thread.
void GameManager::draw()
{
	//Draw to the window as long as it is still open, and the main thread does not want the draw thread to end.
	while(m_window.isOpen() && !m_isWaitingForRenderingEnd)
	{
		//Clear the window of anything that was drawn to it before.
		m_window.clear(sf::Color(0, 0, 20));

		//Allows state to be drawn without read violation from changing state.
		m_stateChangeMutex.lock();

		//Draw the current state to the screen.
		m_window.draw(*m_currentGameState);

		m_stateChangeMutex.unlock();

		//Display the newly drawn elements to the screen.
		m_window.display();
	}

	//Abandon control of rendering, as draw thread is about to end.
	m_window.setActive(false);
}

//Safely starts drawing on the rendering thread.
void GameManager::startRenderThread()
{
	//Deactivate rendering on the main thread, then launch the rendering thread.
	m_window.setActive(false);
	m_renderThread.launch();
}

//Causes the draw thread to safely close; useful for safely closing, or recreating the window.
void GameManager::endRenderThread()
{
	//Set the flag and wait for the rendering thread to close.
	m_isWaitingForRenderingEnd = true;
	m_renderThread.wait();
}
