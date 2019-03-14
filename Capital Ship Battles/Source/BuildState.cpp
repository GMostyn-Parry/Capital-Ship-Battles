/*
 * Author: George Mostyn-Parry
 */
#include "BuildState.hpp"

#include "BattleState.hpp" //The state we will switch to if the user wishes to play singleplayer.
#include "ConnectState.hpp" //The state we will switch to if the user wishes to play multiplayer.

//Define the colour constant for the placeable preview in the build state.
const sf::Color BuildState::COLOUR_PLACEABLE(sf::Color(255, 255, 255, 100));
const sf::Color BuildState::COLOUR_OBSTRUCTED(sf::Color(255, 0, 0, 100));

//Declared in an anonymous namespace to prevent name clashes.
//These have to be global to allow them to be used in the const draw function.
namespace
{
	sf::Mutex turretMutex; //Controls access to turret list, for multithreaded rendering.
	sf::Mutex turretTypeMutex; //Controls access to turret type label.
}

//Basic BuildState constructor.
//	game : The state manager, and holder of high-level information on the game.
BuildState::BuildState(GameManager &game)
	:m_game(game),
	m_hull({0, 0}, 0, {}, m_game.getResourceManager().loadTexture("Assets/hull.png"), m_game.getResourceManager().loadTexture("Assets/turrets.png")),
	m_turretProjType(ProjectileType::LASER),
	m_buildPreview({TurretInfo{m_turretProjType, {0, 0}}, nullptr, m_game.getResourceManager().loadTexture("Assets/turrets.png")}),
	m_singleplayerButton(std::bind(&BuildState::startSingleplayer, this)),
	m_multiplayerButton(std::bind(&BuildState::startMultiplayer, this)),
	m_laserButton(std::bind(&BuildState::setProjectileType, this, ProjectileType::LASER)),
	m_missileButton(std::bind(&BuildState::setProjectileType, this, ProjectileType::MISSILE)),
	m_plasmaButton(std::bind(&BuildState::setProjectileType, this, ProjectileType::PLASMA))
{
	//Load the font used by the GUI elements.
	const sf::Font &arimoFont = *game.getResourceManager().loadFont("Assets/fonts/Arimo-Regular.ttf");

	//Set-up the label that displays the current turret type.
	m_turretTypeText.setFont(arimoFont);
	m_turretTypeText.setString("Laser Turret");

	//Preview should be invisible first, otherwise it will be visible in the default location.
	m_buildPreview.setFillColor(sf::Color::Transparent);

	//Set text, and font, of buttons.
	m_singleplayerButton.setLabel("Local", arimoFont);
	m_multiplayerButton.setLabel("Multi", arimoFont);
	m_laserButton.setLabel("Laser", arimoFont);
	m_missileButton.setLabel("Missile", arimoFont);
	m_plasmaButton.setLabel("Plasma", arimoFont);

	//Add turrets that were in game manager's build list before the player adds their own.
	//I.e. load the configuration the player made first.
	for(auto buildInfo : m_game.turretBuildList)
	{
		//Remove the hull's origin; as the turret local position is calculated from the top-left of the hull,
		//and the construction hull has a centered origin.
		buildInfo.localPosition -= m_hull.getOrigin();
		//Create a new turret with the corrected information.
		m_turretList.push_back(std::make_unique<Turret>(buildInfo, nullptr, m_game.getResourceManager().loadTexture("Assets/turrets.png")));
	}
}

//Passes events to the state, so the state may handle them itself.
//	event : The event for the state to handle.
void BuildState::handleInput(const sf::Event &event)
{
	//Where the mouse was when the event occurred, in global co-ordinates.
	sf::Vector2f globalMousePosition;

	switch(event.type)
	{
		case sf::Event::MouseButtonReleased:
			//Add turret where the user releases their left mouse button.
			switch(event.mouseButton.button) 
			{
				case sf::Mouse::Left:
					globalMousePosition = m_game.getWindow().mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

					//Attempts to add a turret at the location, if the mouse was not released over a button.
					if(!m_singleplayerButton.mouseReleased(globalMousePosition) &&
						!m_multiplayerButton.mouseReleased(globalMousePosition) &&
						!m_laserButton.mouseReleased(globalMousePosition) && 
						!m_missileButton.mouseReleased(globalMousePosition) && 
						!m_plasmaButton.mouseReleased(globalMousePosition))
					{
						addTurret();
					}

					break;
				//Clear the turrets intersecting the preview turret, when the right mouse button is released.
				case sf::Mouse::Right:
					clearArea();

					break;
			}
			break;
		//Move the ghost, and determine if it is in a valid position.
		case sf::Event::MouseMoved:
			//Convert event members into a global mouse position, and move the preview ghost there.
			globalMousePosition = m_game.getWindow().mapPixelToCoords({event.mouseMove.x, event.mouseMove.y});
			m_buildPreview.setPosition(globalMousePosition);

			//Set the fill colour to the placeable colour; if is in not obstructed, otherwise set it to the obstructed colour.
			m_buildPreview.setFillColor(isValidPlacement(m_buildPreview) ? COLOUR_PLACEABLE : COLOUR_OBSTRUCTED);
			break;
		case sf::Event::KeyPressed:
			switch(event.key.code)
			{
				//Changes to the laser turret when the number 1 key is pressed.
				case sf::Keyboard::Num1:
					setProjectileType(ProjectileType::LASER);

					break;
				//Changes to the missile turret when the number 2 key is pressed.
				case sf::Keyboard::Num2:
					setProjectileType(ProjectileType::MISSILE);

					break;
				//Changes to the plasma turret when the number 3 key is pressed.
				case sf::Keyboard::Num3:
					setProjectileType(ProjectileType::PLASMA);

					break;
				//Build a debug ship - full turrets - when the F3 key is pressed.
				case sf::Keyboard::F3:
					//Add as many turrets as possible on the y-axis.
					for(float y = m_hull.getGlobalBounds().top; y < m_hull.getGlobalBounds().top + m_hull.getGlobalBounds().height; y += 42)
					{
						//Add as many turrets as possible on the x-axis.
						for(float x = m_hull.getGlobalBounds().left; x < m_hull.getGlobalBounds().left + m_hull.getGlobalBounds().width; x += 42)
						{
							//Set ghost to position, then add the turret to the ship.
							m_buildPreview.setPosition(x, y);
							addTurret();
						}
					}

					break;
			}
			break;
		//Make the preview transparent when the mouse leaves the window.
		case sf::Event::MouseLeft:
			m_buildPreview.setFillColor(sf::Color::Transparent);

			break;
	}
}

//Processes the state logic to move the state forward a tick.
//	deltaTime : The amount of time that has passed since the last update.
void BuildState::update(const sf::Time &deltaTime)
{}

//Draws all renderable elements of the state to the screen.
//	target : What we will be drawing onto.
//	states : Visual manipulations to the elements that are being drawn.
void BuildState::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	//Draw the ship hull.
	target.draw(m_hull, states);

	//Lock turret list for drawing.
	turretMutex.lock();

	//Draw the turrets.
	for(const auto &turret : m_turretList)
	{
		target.draw(*turret, states);
	}

	turretMutex.unlock();

	//Draw the preview of the next turret.
	target.draw(m_buildPreview, states);

	//Draw the GUI elements.
	target.draw(m_singleplayerButton, states);
	target.draw(m_multiplayerButton, states);
	target.draw(m_laserButton, states);
	target.draw(m_missileButton, states);
	target.draw(m_plasmaButton, states);

	//Lock turret type label so it may be drawn to the screen.
	turretTypeMutex.lock();

	target.draw(m_turretTypeText, states);

	turretTypeMutex.unlock();
}

//Update the state's view, i.e. fix the GUI, and other elements, from a window resize.
void BuildState::updateView()
{
	//Size of the window we are rendering to.
	const sf::Vector2f windowSize = sf::Vector2f(m_game.getWindow().getSize());

	//Place ship hull at centre of the screen; ships have a centred origin.
	sf::Vector2f hullPosition = windowSize / 2.f;
	
	//Place the finish button in the bottom-right of the screen.
	sf::Vector2f singleplayerButtonPosition = windowSize - m_singleplayerButton.getSize();
	//Place the finish button directly above the singleplayer button.
	sf::Vector2f multiplayerButtonPosition = singleplayerButtonPosition - sf::Vector2f(0, m_multiplayerButton.getSize().y);

	//Place the laser select button in the top-left of the screen.
	sf::Vector2f laserButtonPosition = sf::Vector2f(0, 0);
	//Place the missile select button directly below the laser select button.
	sf::Vector2f missileButtonPosition = laserButtonPosition + sf::Vector2f(0, m_missileButton.getSize().y);
	//Place the plasma select button directly below the missile select button.
	sf::Vector2f plasmaButtonPosition = missileButtonPosition + sf::Vector2f(0, m_laserButton.getSize().y);

	//Update the turret positions.
	for(auto &turret : m_turretList)
	{
		turret->setPosition(m_game.getWindow().mapPixelToCoords(sf::Vector2i(turret->getPosition() - m_hull.getPosition() + hullPosition)));
	}

	//Place the GUI elements mapped to global co-ordinates.
	m_hull.setPosition(m_game.getWindow().mapPixelToCoords(sf::Vector2i(hullPosition), m_game.getWindow().getView()));
	m_singleplayerButton.setPosition(m_game.getWindow().mapPixelToCoords(sf::Vector2i(singleplayerButtonPosition)));
	m_multiplayerButton.setPosition(m_game.getWindow().mapPixelToCoords(sf::Vector2i(multiplayerButtonPosition)));
	m_laserButton.setPosition(m_game.getWindow().mapPixelToCoords(sf::Vector2i(laserButtonPosition)));
	m_missileButton.setPosition(m_game.getWindow().mapPixelToCoords(sf::Vector2i(missileButtonPosition)));
	m_plasmaButton.setPosition(m_game.getWindow().mapPixelToCoords(sf::Vector2i(plasmaButtonPosition)));

	//Call specific function to update position of turret type label.
	updateTurretTypeText();
}

//Changes the projectile type of the turret to be added to the projectile type passed.
//	projType : The new projectile type of the turret to be added.
void BuildState::setProjectileType(ProjectileType projType)
{
	//Update the projectile type.
	m_turretProjType = projType;

	//Get the turret's colour before it is lost to the assignment.
	sf::Color oldColour = m_buildPreview.getFillColor();

	//Changes preview (ghost) turret to the new projectile type by constructing a new object.
	m_buildPreview = Turret({projType, m_buildPreview.getPosition()}, nullptr, m_game.getResourceManager().loadTexture("Assets/turrets.png"));
	//Keep the colour of the turret.
	m_buildPreview.setFillColor(oldColour);

	//Lock turret type label for write access.
	turretTypeMutex.lock();

	switch(projType)
	{
		case ProjectileType::LASER:
			//Update display text; we need to adjust the position to keep it centred, as the width will have changed.
			m_turretTypeText.setString("Laser Turret");
			updateTurretTypeText();
			
			break;
		case ProjectileType::MISSILE:
			//Update display text; we need to adjust the position to keep it centred, as the width will have changed.
			m_turretTypeText.setString("Missile Turret");
			updateTurretTypeText();

			break;
		case ProjectileType::PLASMA:
			//Update display text; we need to adjust the position to keep it centred, as the width will have changed.
			m_turretTypeText.setString("Plasma Turret");
			updateTurretTypeText();

			break;
	}

	turretTypeMutex.unlock();
}

//Adds a turret to the ship under construction, where the preview is.
//Returns whether the turret was successfully placed.
bool BuildState::addTurret()
{
	//Create the turret if it is a valid placement.
	if(isValidPlacement(m_buildPreview))
	{
		//Lock turret list for writing.
		turretMutex.lock();

		//Create a new unique pointer to the turret, and store it in the list.
		m_turretList.push_back(std::make_unique<Turret>(Turret({m_turretProjType, m_buildPreview.getPosition()}, &m_hull.getTransform(), m_game.getResourceManager().loadTexture("Assets/turrets.png"))));

		turretMutex.unlock();

		//Return that the turret was placed.
		return true;
	}

	//Return that the turret was not placed.
	return false;
}

//Removes all turrets that are colliding with the preview.
void BuildState::clearArea()
{
	//Lock turret list for write access.
	turretMutex.lock();

	//Find, and remove, all turrets that intersect with the preview.
	for(auto it = m_turretList.begin(); it != m_turretList.end();)
	{
		//Erase the turret if its global bounds intersect with the preview's global bounds.
		if((*it)->getGlobalBounds().intersects(m_buildPreview.getGlobalBounds()))
		{
			it = m_turretList.erase(it);
		}
		//Otherwise, simply increment the iterator.
		else
		{
			++it;
		}
	}

	turretMutex.unlock();

	//Show the turret is no longer obstructed.
	m_buildPreview.setFillColor(COLOUR_PLACEABLE);
}

//Returns the build info of the turrets attached to the ship.
std::vector<TurretInfo> BuildState::getTurretBuildInfo() const
{
	//Stores information on how to build every turret in the construction manager.
	std::vector<TurretInfo> turretList;

	//Creates build information for every turret.
	for(const auto &turret : m_turretList)
	{
		TurretInfo newInfo = turret->getTurretInfo();
		//Remove ship position from turret to get local position of turret, relative to the ship.
		newInfo.localPosition -= m_hull.getPosition() - m_hull.getOrigin();
		turretList.push_back(newInfo);
	}

	return turretList;
}

//Returns whether the placeable can be placed at this location.
bool BuildState::isValidPlacement(const sf::RectangleShape &placeable)
{
	bool isValid = true;

	//We want the object to collide with the ship, as the object should be attached to the ship.
	if(m_hull.collide(placeable.getPosition()))
	{
		//Iterator for looping through turrets vector.
		auto it = m_turretList.begin();
		//Search for turret that intersects with the object we are trying to place.
		while(isValid && it != m_turretList.end())
		{
			//Object is not valid if an already placed turret intersects with it.
			if((*it)->getGlobalBounds().intersects(placeable.getGlobalBounds()))
			{
				isValid = false;
			}

			it++;
		}
	}
	//Otherwise, object is not attached and thus invalid.
	else
	{
		isValid = false;
	}

	return isValid;
}

//Places the label denoting the turret type text in the centre-top of the screen.
void BuildState::updateTurretTypeText()
{
	//Place the turret type text at the centre of the top of the screen.
	sf::Vector2f turretTypePosition = sf::Vector2f((m_game.getWindow().getSize().x - m_turretTypeText.getGlobalBounds().width) / 2.f, 0);
	m_turretTypeText.setPosition(m_game.getWindow().mapPixelToCoords(sf::Vector2i(turretTypePosition)));
}

//Switches directly to the battle state.
void BuildState::startSingleplayer()
{
	//Tell the game manager the configuration of turrets the player made.
	m_game.turretBuildList = getTurretBuildInfo();
	//Change to battle state.
	m_game.setState(std::make_unique<BattleState>(m_game, false));
}

//Switches to the connect state, so we can attempt to join another user.
void BuildState::startMultiplayer()
{
	//Tell the game manager the configuration of turrets the player made.
	m_game.turretBuildList = getTurretBuildInfo();
	//Change to connect state.
	m_game.setState(std::make_unique<ConnectState>(m_game));
}
