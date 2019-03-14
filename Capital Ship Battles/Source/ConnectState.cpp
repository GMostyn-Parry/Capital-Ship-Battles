/*
 * Author: George Mostyn-Parry
 */
#include "ConnectState.hpp"

#include "BattleState.hpp" //The state we will change to after this state.
#include "BuildState.hpp" //The state the user may return to.

 //Declared in an anonymous namespace to prevent name clashes.
 //These have to be global to allow them to be used in the const draw function.
namespace
{
	sf::Mutex labelMutex; //Controls access to label list, for multithreaded rendering.
	sf::Mutex buttonMutex; //Controls access to button list, for multithreaded rendering.

	sf::Mutex peerFlagMutex; //Controls access to the peer flag, for multithreaded connection handling.
}

//Basic ConnectState constructor.
//	game : The state manager, and holder of high-level information on the game.
ConnectState::ConnectState(GameManager &game)
	:m_game(game),
	m_leaveButton(std::bind(&ConnectState::startBuild, this))
{
	//Font used by the GUI elements.
	const sf::Font &arimoFont = *m_game.getResourceManager().loadFont("Assets/fonts/Arimo-Regular.ttf");

	//Set text and font of the leave button.
	m_leaveButton.setLabel("Leave", arimoFont);

	//Add a new label to the state asking the user if they want to host.
	m_labelList.emplace_back("Would you like to host a server, or join a server?", arimoFont);
	//Change the newly emplaced label's origin to its centre.
	m_labelList[0].setOrigin(m_labelList[0].getGlobalBounds().width / 2.f, m_labelList[0].getGlobalBounds().height / 2.f);

	//Add the hosting button to the state.
	m_buttonList.emplace_back(std::bind(&ConnectState::enterHostState, this), "Host", arimoFont);
	//Centre the button's origin.
	m_buttonList[0].setOrigin(m_buttonList[0].getSize() / 2.f);

	//Add the joining button to the state.
	m_buttonList.emplace_back(std::bind(&ConnectState::enterJoinState, this), "Join", arimoFont);
	//Centre the button's origin.
	m_buttonList[1].setOrigin(m_buttonList[1].getSize() / 2.f);
}

//ConnectState destructor.
ConnectState::~ConnectState()
{
	//Ends the connection thread if there is a thread to end, and we don't have a peer; i.e the successful state.
	if(m_connectThread && !m_hasPeer)
	{
		//Stops any connection attempts.
		m_game.getNetworkManager().closeAllConnections();

		//Wait for connection thread to end.
		m_connectThread->wait();
	}
}

//Passes events to the state, so the state may handle them itself.
//	event : The event for the state to handle.
void ConnectState::handleInput(const sf::Event &event)
{
	switch(event.type)
	{
		//Attempt to find which button the mouse was released over.
		case sf::Event::MouseButtonReleased:
		{
			//Where the mouse cursor was released.
			sf::Vector2f globalMousePosition = m_game.getWindow().mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});

			//Find, and activate, the button we released the mouse over.
			for(auto &button : m_buttonList)
			{
				//Break the loop if we found the button we released the mouse over.
				if(button.mouseReleased(globalMousePosition)) break;
			}

			//Attempt to activate the leave button.
			m_leaveButton.mouseReleased(globalMousePosition);

			break;
		}
		case sf::Event::KeyPressed:
			//Attempt to join server with the entered IP if we are in the joining internal state and the return key was pressed.
			//But not Alt+Enter, as that is fullscreen toggle.
			if(m_isJoining && event.key.code == sf::Keyboard::Return && !event.key.alt)
			{
				m_connectThread->launch();
			}

			break;
		case sf::Event::TextEntered:
			//Enter text into IP text entry, if we are in the joining state.
			if(m_isJoining)
			{
				//Lock label list for write access.
				labelMutex.lock();

				sf::Text &textEntry = m_labelList[1];

				//Remove character at end, if backspace was pressed.
				if(event.text.unicode == 8)
				{
					textEntry.setString(textEntry.getString().substring(0, textEntry.getString().getSize() - 1));
				}
				//Otherwise, append the pressed character.
				else
				{
					textEntry.setString(textEntry.getString() + event.text.unicode);
				}

				//Adjust text origin to new centre.
				textEntry.setOrigin(textEntry.getGlobalBounds().width / 2.f, textEntry.getGlobalBounds().height / 2.f);

				labelMutex.unlock();
			}

			break;
	}
}

//Processes the state logic to move the state forward a tick.
//	deltaTime : The amount of time that has passed since the last update.
void ConnectState::update(const sf::Time &deltaTime)
{
	//Lock access to peer flag, for read access.
	peerFlagMutex.lock();

	//Start the battle when we successfully get a peer.
	if(m_hasPeer) startBattle();

	peerFlagMutex.unlock();
}

//Draws all renderable elements of the state to the screen.
//	target : What we will be drawing onto.
//	states : Visual manipulations to the elements that are being drawn.
void ConnectState::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	//Lock button list for read access.
	buttonMutex.lock();

	//Draw all of the buttons to the screen.
	for(const auto &button : m_buttonList)
	{
		target.draw(button, states);
	}

	//Draw the leave button.
	target.draw(m_leaveButton, states);

	buttonMutex.unlock();
	//Lock label list for read access.
	labelMutex.lock();

	//Draw all of the labels to the screen.
	for(const auto &label : m_labelList)
	{
		target.draw(label, states);
	}

	labelMutex.unlock();
}

//Update the state's view, i.e. fix the GUI, and other elements, from a window resize.
void ConnectState::updateView()
{
	//Lock label and button lists for write access.
	labelMutex.lock();
	buttonMutex.lock();

	//The centre of the window.
	sf::Vector2f windowCentre = sf::Vector2f(m_game.getWindow().getSize()) / 2.f;
	//How many rows of GUI elements we will be displaying; the buttons are all on one row.
	unsigned int rows = m_labelList.size() + (m_buttonList.size() != 0 ? 1 : 0);
	//How far apart each row is from each other.
	float rowSeparation = m_game.getWindow().getSize().y / (rows + 1.f);

	//Place each label on its own row.
	for(unsigned int i = 0; i < m_labelList.size(); i++)
	{
		m_labelList[i].setPosition(m_game.getWindow().mapPixelToCoords(sf::Vector2i(static_cast<int>(windowCentre.x), static_cast<int>(rowSeparation * (i + 1)))));
	}

	//How far apart each column(button) is from each other.
	float columnSeparation = m_game.getWindow().getSize().x / (m_buttonList.size() + 1.f);

	//Place the buttons on the last row, with even spacing.
	for(unsigned i = 0; i < m_buttonList.size(); i++)
	{
		m_buttonList[i].setPosition(m_game.getWindow().mapPixelToCoords(sf::Vector2i(static_cast<int>(columnSeparation * (i + 1)), static_cast<int>(rowSeparation * rows))));
	}

	//Place the leave button at the top-left of the window.
	m_leaveButton.setPosition(m_game.getWindow().mapPixelToCoords(sf::Vector2i(0, 0)));

	buttonMutex.unlock();
	labelMutex.unlock();
}

//Changes to the hosting state of the connect state.
void ConnectState::enterHostState()
{
	//Lock button mutex for write access, so we can clear the buttons.
	buttonMutex.lock();
	m_buttonList.clear();
	buttonMutex.unlock();

	//Lock label mutex for write access.
	labelMutex.lock();

	//Get label that exists and simply adjust it to suit our needs; change the text, and alter its origin.
	sf::Text &label = m_labelList.back();
	label.setString("Waiting for a player to join.");
	label.setOrigin(label.getGlobalBounds().width / 2.f, label.getGlobalBounds().height / 2.f);

	labelMutex.unlock();

	//Update view to match new GUI.
	updateView();

	//Launch waiting for client on a separate thread.
	m_connectThread = std::make_unique<sf::Thread>(&ConnectState::waitForClient, this);
	m_connectThread->launch();
}

//Changes to the joining state of the connect state.
void ConnectState::enterJoinState()
{
	//The centre of the window.
	const sf::Vector2f windowCentre = m_game.getWindow().getView().getCenter();
	//We want to attempt to join the host on a separate thread, so we don't stop input event handling.
	m_connectThread = std::make_unique<sf::Thread>(&ConnectState::attemptJoin, this);

	//Lock label mutex for write access.
	labelMutex.lock();

	//Get label that exists and simply adjust it to suit our needs; change text, and alter origin.
	sf::Text &label = m_labelList.back();
	label.setString("Enter the IP of the server you wish to join:");
	label.setOrigin(label.getGlobalBounds().width / 2.f, label.getGlobalBounds().height / 2.f);

	//Make a label identical to the existing label to the list; this will represent the text entry.
	m_labelList.push_back(label);

	//The new label that will represent the text entry; we adjust the origin after placing it on the list.
	sf::Text &newLabel = m_labelList.back();
	newLabel.setString("");
	newLabel.setOrigin(newLabel.getGlobalBounds().width / 2.f, newLabel.getGlobalBounds().height / 2.f);

	labelMutex.unlock();

	//Lock button mutex for write access.
	buttonMutex.lock();

	//Clear button list for new buttons.
	m_buttonList.clear();

	//Add a button thats allow the user to click it to confirm the entered IP.
	m_buttonList.emplace_back(std::bind(&sf::Thread::launch, m_connectThread.get()), "Join", *m_labelList[0].getFont(), windowCentre + sf::Vector2f(0, 100));
	//Set origin of newly added button to its own centre.
	m_buttonList[0].setOrigin(m_buttonList[0].getSize() / 2.f);

	buttonMutex.unlock();

	//Update view to match new GUI.
	updateView();

	//Flag we are in the joining state.
	m_isJoining = true;
}

//Waits for the server to receive a client before switching to the battle state.
void ConnectState::waitForClient()
{
	//Start the battle, if we managed to set up the server.
	if(m_game.getNetworkManager().hostServer())
	{
		//Lock access to peer flag, for write access.
		peerFlagMutex.lock();

		//Flag to start the battle on the main thread.
		m_hasPeer = true;

		peerFlagMutex.unlock();
	}
}

//Attempt to join a server with the currently entered string input.
void ConnectState::attemptJoin()
{
	//Erase failure label, if we have already placed it, so there is a noticeable change.
	if(m_labelList.size() == 3)
	{
		//Lock label list  for write access.
		labelMutex.lock();

		//Erase failure label, which is in position 2.
		m_labelList.erase(m_labelList.begin() + 2);

		labelMutex.unlock();

		//Update view to match new GUI.
		updateView();
	}

	//Flag the battle to start, if we successfully joined on the passed IP.
	if(m_game.getNetworkManager().joinServer(m_labelList[1].getString()))
	{
		//Lock access to peer flag, for write access.
		peerFlagMutex.lock();

		m_hasPeer = true;

		peerFlagMutex.unlock();
	}
	//Otherwise, ask the user to check input.
	else
	{
		//Lock label mutex for write access.
		labelMutex.lock();

		//Create failure message.
		m_labelList.emplace_back("Failed to connect. Ensure you entered a valid IP.", *m_labelList[0].getFont());
		m_labelList[2].setOrigin(m_labelList[2].getGlobalBounds().width / 2.f, m_labelList[2].getGlobalBounds().height / 2.f);

		labelMutex.unlock();

		//Update view to match new GUI.
		updateView();
	}
}

//Changes the current state to the build state.
void ConnectState::startBuild()
{
	m_game.setState(std::make_unique<BuildState>(m_game));
}

//Changes the current state to a networked battle.
void ConnectState::startBattle()
{
	m_game.setState(std::make_unique<BattleState>(m_game, true));
}
