/*
 * Author: George Mostyn-Parry
 */
#include "BattleState.hpp"

#include "BuildState.hpp" //The state we want to change to when the battle ends.

std::vector<ShotInfo> *Turret::s_fireList; //List that turrets use to queue shots.

//Declared in an anonymous namespace to prevent name clashes.
//These have to be global to allow them to be used in the const draw function.
namespace
{
	sf::Mutex shipMutex; //Controls access to the ship list.
	sf::Mutex projMutex; //Controls access to the projectile list.
}

//Basic BattleState constructor.
//	game : The state manager, and holder of high-level information on the game.
//	isMultiplayer : Whether this battle is being networked.
BattleState::BattleState(GameManager &game, bool isMultiplayer)
	:m_game(game), m_networkThread(&NetworkManager::receive, &m_game.getNetworkManager()),
	m_viewBounds({0, 0, 4000, 4000}), m_gameView(m_game.getWindow().getView()),
	areaBorder(sf::Vector2f(m_viewBounds.width, m_viewBounds.height))
{
	//Allow turrets to queue shots directly to the projectile creation list.
	Turret::s_fireList = &m_readyToFire;
	
	//The centre of the playable area.
	sf::Vector2f centreField = {m_viewBounds.width / 2.f, m_viewBounds.height / 2.f};
	//How much we are going to offset both ships from the centre.
	sf::Vector2f shipOffset = {200, 200};

	//Create empty vectors on both indices.
	m_shipList[0] = std::vector<std::unique_ptr<Ship>>();
	m_shipList[1] = std::vector<std::unique_ptr<Ship>>();

	//Launch the networking thread, so both players can receive each other's ships; if we are in multiplayer mode.
	if(isMultiplayer)
	{
		//Create the player's ship in the top-left if they are the host.
		if(m_game.getNetworkManager().isHost())
		{
			createShip(0, {1800, 1800}, 45, m_game.turretBuildList);
		}
		//Otherwise, create it in the bottom-right.
		else
		{
			createShip(0, {2200, 2200}, 225, m_game.turretBuildList);
		}

		//Set the variables needed by the network manager to network this battle.
		m_game.getNetworkManager().setBattle(this);
		m_game.getNetworkManager().setTurretList(m_game.turretBuildList);

		//Launch the networking thread, so we may receive packets.
		m_networkThread.launch();

		//Send the local player's ship to their peer.
		m_game.getNetworkManager().sendShip();
	}
	//Otherwise, just create two ships with the same configuration.
	else
	{
		createShip(0, centreField - shipOffset, 45, m_game.turretBuildList);
		createShip(1, centreField + shipOffset, 225, m_game.turretBuildList);
	}

	//Set the centre of the view in the centre of the playing field.
	m_gameView.setCenter(centreField);

	//Make the border's internal colour fully transparent.
	areaBorder.setFillColor(sf::Color(0, 0, 0, 0));
	//Give it an outline, so it can be seen.
	areaBorder.setOutlineThickness(-16);
}

//BattleState destructor.
BattleState::~BattleState()
{
	//Close all connections, before ending the battle.
	m_game.getNetworkManager().closeAllConnections();
	//Wait for the networking thread to end.
	m_networkThread.wait();
}

//Passes events to the state, so the state may handle them itself.
//	event : The event for the state to handle.
void BattleState::handleInput(const sf::Event &event)
{
	sf::Vector2f mouseGlobalPosition;
	sf::Packet packet;

	//Handle each type of event.
	switch(event.type)
	{
		//Issue move command to ship {1, 0}, if the middle mouse was pressed.
		case sf::Event::MouseButtonPressed:
			mouseGlobalPosition = m_game.getWindow().mapPixelToCoords({event.mouseButton.x, event.mouseButton.y}, m_gameView);

			switch(event.mouseButton.button)
			{
				
				//Issue a move command if the right mouse button was pressed.
				case sf::Mouse::Right:
					issueMoveCommand(0, 0, mouseGlobalPosition);

					//Package the command to move for transport.
					packet << std::underlying_type_t<PacketType>(PacketType::MOVE);
					packet << 0;
					packet << mouseGlobalPosition.x;
					packet << mouseGlobalPosition.y;

					//Send the move command over the network.
					m_game.getNetworkManager().send(packet);

					break;
				//Issue an attack command on the left mouse button being pressed.
				case sf::Mouse::Left:
					issueFireCommand(0, 0, mouseGlobalPosition, 1);

					//Package the command to attack.
					packet << std::underlying_type_t<PacketType>(PacketType::FIRE);
					packet << 0;
					packet << mouseGlobalPosition.x;
					packet << mouseGlobalPosition.y;

					//Send the attack command.
					m_game.getNetworkManager().send(packet);

					break;
			}

			break;
		//Change the view when the mouse wheel is scrolled
		case sf::Event::MouseWheelScrolled: 
			//The global co-ordinates of the mouse when the mouse wheel was scrolled.
			mouseGlobalPosition = m_game.getWindow().mapPixelToCoords({event.mouseWheelScroll.x, event.mouseWheelScroll.y}, m_gameView);

			//Zoom in when the mousewheel is scrolled up, and it will not make the view too small.
			if(event.mouseWheelScroll.delta == 1 && m_gameView.getSize().x > m_viewBounds.width / 8)
			{
				//Zoom in.
				m_gameView.zoom(0.5);
				//Move the view to keep the zoomed-in area in view.
				m_gameView.move((mouseGlobalPosition.x - m_gameView.getCenter().x) / 2, (mouseGlobalPosition.y - m_gameView.getCenter().y) / 2);
			}
			//Zoom out when the mousewheel scrolls down, and the view won't become too big.
			else if(event.mouseWheelScroll.delta == -1 && m_gameView.getSize().x < m_viewBounds.width)
			{
				//Zoom out.
				m_gameView.zoom(2);

				//The mouse position after the view is zoomed out, relative to the game view.
				sf::Vector2f postMousePosition = m_game.getWindow().mapPixelToCoords(sf::Mouse::getPosition(m_game.getWindow()), m_gameView);
				//Move the view, so the mouse cursor is still over the pixel it was highlighting before.
				m_gameView.move((mouseGlobalPosition.x - postMousePosition.x), (mouseGlobalPosition.y - postMousePosition.y));
			}

			//Set the view to the horizontal centre, if the view is too large in width.
			if(m_gameView.getSize().x >= m_viewBounds.width)
			{
				m_gameView.setCenter(m_viewBounds.width / 2.f, m_gameView.getCenter().y);
			}
			//Move the view back into the bounds if it goes too far left.
			else if(m_gameView.getCenter().x - m_gameView.getSize().x / 2.f < m_viewBounds.left)
			{
				m_gameView.setCenter(m_viewBounds.left + m_gameView.getSize().x / 2.f, m_gameView.getCenter().y);
			}
			//Move the view back into the bounds if it goes too far right.
			else if(m_gameView.getCenter().x + m_gameView.getSize().x / 2.f > m_viewBounds.left + m_viewBounds.width)
			{
				m_gameView.setCenter(m_viewBounds.left + m_viewBounds.width - m_gameView.getSize().x / 2.f, m_gameView.getCenter().y);
			}

			//Set the view to the vertical centre, if the view is too large in height.
			if(m_gameView.getSize().y >= m_viewBounds.height)
			{
				m_gameView.setCenter(m_gameView.getCenter().x, m_viewBounds.height / 2.f);
			}
			//Move the view back into the bounds if it goes too far up.
			else if(m_gameView.getCenter().y - m_gameView.getSize().y / 2.f < m_viewBounds.top)
			{
				m_gameView.setCenter(m_gameView.getCenter().x, m_viewBounds.top + m_gameView.getSize().y / 2.f);
			}
			//Move the view back into the bounds if it goes too far down.
			else if(m_gameView.getCenter().y + m_gameView.getSize().y / 2.f > m_viewBounds.top + m_viewBounds.height)
			{
				m_gameView.setCenter(m_gameView.getCenter().x, m_viewBounds.top + m_viewBounds.height - m_gameView.getSize().y / 2.f);
			}

			break;
	}
}

//Processes the state logic to move the state forward a tick.
//	deltaTime : The amount of time that has passed since the last update.
void BattleState::update(const sf::Time &deltaTime)
{
	//Lock projectile list for write access.
	projMutex.lock();

	//Create every projectile that has been queued.
	for(const auto &fireInfo : m_readyToFire)
	{
		createProjectile(fireInfo);
	}

	projMutex.unlock();

	//Clear the list, as the projectiles have been created.
	m_readyToFire.clear();

	//Process the projectiles this tick.
	resolveProjectiles(deltaTime);

	//Lock ship list for write access.
	shipMutex.lock();

	//Process every ship on each layer for this tick.
	for(const auto &battleLayer : m_shipList)
	{
		for(const auto &ship : battleLayer)
		{
			ship->update(deltaTime);
		}
	}

	shipMutex.unlock();

	//Go to the build state if the battle is finished; i.e. either team has no ships.
	//We can't kill the state during the collision as the stack needs to unwind,
	//and the update may try to work with corrupt data if we kill the state too soon.
	if(m_isFinished)
	{
		changeToBuildState();
	}
}

//Draws all renderable elements of the state to the screen.
//	target : What we will be drawing onto.
//	states : Visual manipulations to the elements that are being drawn.
void BattleState::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	//The view of the target when it was passed to the draw function.
	sf::View targetView = target.getView();

	//We want the game elements relative to the game view.
	target.setView(m_gameView);

	//Draw border below everything else.
	target.draw(areaBorder);

	//Lock ship list for rendering.
	shipMutex.lock();

	//Draw ships, on all layers, onto the render target.
	for(const auto &battleLayer : m_shipList)
	{
		//Draw each ship on the current layer.
		for(const auto &ship : battleLayer)
		{
			target.draw(*ship, states);
		}
	}

	shipMutex.unlock();

	//Lock projectile list for rendering.
	projMutex.lock();

	//Draw every projectile onto the render target.
	for(const auto &proj : m_projList)
	{
		target.draw(*proj, states);
	}

	projMutex.unlock();

	//Restore the target's view.
	target.setView(targetView);
}

//Update the state's view, i.e. fix the GUI, and other elements, from a window resize.
void BattleState::updateView()
{
	m_gameView.setSize(sf::Vector2f(m_game.getWindow().getSize()));
}

//Creates a ship with the passed information.
//	team : The team the ship belongs to.
//	position : Where the ship should start on creation.
//	angle : The rotation of the ship on creation.
//	turretBuildList : The turrets the ship should start with.
void BattleState::createShip(unsigned int team, const sf::Vector2f &position, float angle, const std::vector<TurretInfo> &turretBuildList)
{
	//Lock ship list for write access.
	shipMutex.lock();

	//Create a new unique pointer that stores a ship.
	m_shipList[team].push_back(std::make_unique<Ship>(position, angle, turretBuildList,
		m_game.getResourceManager().loadTexture("Assets/hull.png"), m_game.getResourceManager().loadTexture("Assets/turrets.png")));

	shipMutex.unlock();
}

//Passes a move command to the specified ship.
//	shipLayer : The layer the ship is on; i.e. which team.
//	shipID : The ID of the ship in the team.
//	destination : Where the ship is being told to head to.
void BattleState::issueMoveCommand(unsigned int shipLayer, unsigned int shipID, const sf::Vector2f &destination)
{
	m_shipList[shipLayer][shipID]->moveCommand(destination);
}

//Passes a fire command to the specified ship.
//	shipLayer : The layer the ship is on; i.e. which team.
//	shipID : The ID of the ship in the team.
//	target : Where the ship is being told to shoot at.
//	targetLayer : Which layer the ship is being told to shoot on.
void BattleState::issueFireCommand(unsigned int shipLayer, unsigned int shipID, const sf::Vector2f &target, unsigned int targetLayer)
{
	m_shipList[shipLayer][shipID]->fireCommand(target, targetLayer);
}

//Creates a projectile with the passed information.
//	info : The information used to create the projectile.
void BattleState::createProjectile(const ShotInfo &info)
{
	projMutex.lock();

	m_projList.push_back(std::make_unique<Projectile>(info));

	projMutex.unlock();
}

//Processes a single tick for all projectiles.
//	deltaTime : The amount of time that has passed since the last update.
void BattleState::resolveProjectiles(const sf::Time &deltaTime)
{
	///Too many projectiles can cause the draw thread to starve.
	//Lock projectile list for write access; necessary here as we might delete the projectile and change the list.
	projMutex.lock();

	//Iterate through projectiles and resolve the current tick; delete projectiles that are finished.
	for(auto it = m_projList.begin(); it != m_projList.end();)
	{
		//Process a tick for the projectile.
		(*it)->update(deltaTime);

		//Remove the projectile if it is finished, it collided with something, or it is out of bounds.
		if((*it)->requiresCleanup() || collide(*it, deltaTime) || !(*it)->getGlobalBounds().intersects(m_viewBounds))
		{
			it = m_projList.erase(it);
		}
		else
		{
			it++;
		}
	}

	projMutex.unlock();
}

//Determines if the projectile collided with anything.
//	proj : The projectile we are checking collisions for.
//	deltaTime : The amount of time that has passed since the last update.
//Returns whether a collision occurred.
bool BattleState::collide(const std::unique_ptr<Projectile> &proj, const sf::Time &deltaTime)
{
	bool wasCollision = false; //Whether there was a collision.
	unsigned int projLayer = proj->getLayer(); //Layer the projectile is on.
	auto it = m_shipList[projLayer].begin(); //Iterator through ship list on projectile's layer.

	//Determines if there was a collision between the projectile, and any of the ships on the same layer.
	//Loops ends when a collision occurs, or there are no more ships to check.
	while(!wasCollision && it != m_shipList[projLayer].end())
	{
		//Marks there was a collision, and potentially removes ship, if there was a collision.
		if((*it)->collide(*proj, deltaTime))
		{
			//Removes the ship from the game if it died from the shot.
			if((*it)->requiresCleanup())
			{
				shipMutex.lock();

				it = m_shipList[projLayer].erase(it);

				shipMutex.unlock();

				//Flag the battle as finished, if the team that lost the ship no longer has any remaining ships.
				m_isFinished = m_shipList[projLayer].size() == 0;
			}

			wasCollision = true;
		}
		else
		{
			it++;
		}		
	}

	return wasCollision;
}

//Ends the battle state, and proceeds to the build state.
void BattleState::changeToBuildState()
{
	m_game.setState(std::make_unique<BuildState>(m_game));
}
