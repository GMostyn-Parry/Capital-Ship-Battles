/*
 * Author: George Mostyn-Parry
 */
#include "Ship.hpp"

#include "CSB_Functions.hpp" //For rotating to face the movement destination.

 //Declared in an anonymous namespace to prevent name clashes.
 //These have to be global to allow them to be used in the const draw function.
namespace
{
	sf::Mutex turretMutex; //Controls access to the turret list.
}

//Construct ship with passed parameters.
//	position : Position the ship starts at.
//	angle : Angle the ship starts at.
//	turretList : List of turrets the ship starts with.
//	hullTexture : Texture of the ship hull.
//	turretAtlasTexture : Texture atlas for the turrets.
Ship::Ship(const sf::Vector2f &position, float angle, const std::vector<TurretInfo> &turretList, const sf::Texture *hullTexture, const sf::Texture *turretAtlasTexture)
{
	setPosition(position);
	setRotation(angle);
	//Set texture, and set the origin to the centre of the texture.
	setTexture(*hullTexture);
	setOrigin(sf::Vector2f(getLocalBounds().width, getLocalBounds().height) / 2.f);

	//Create the image key as a quarter of the size of the texture, so attacks are more impactful.
	m_keyImage.create(getTexture()->getSize().x / KEY_SIZE_FACTOR, getTexture()->getSize().y / KEY_SIZE_FACTOR, sf::Color(0, 0, 0, 0));

	//Image of texture, so we can read its pixels..
	sf::Image textureImage = getTexture()->copyToImage();

	//Colour all non-transparent pixel blocks as white; in pixel groups as defined by the KEY_SIZE_FACTOR.
	for(unsigned int y = 0; y < getTexture()->getSize().y; y += KEY_SIZE_FACTOR)
	{
		for(unsigned int x = 0; x < getTexture()->getSize().x; x += KEY_SIZE_FACTOR)
		{
			//Whether there is an opaque pixel in this pixel block.
			bool hasOpaquePixel = false;

			//Check for an opaque pixel in this pixel block.
			for(unsigned int j = 0; j < KEY_SIZE_FACTOR; ++j)
			{
				for(unsigned int i = 0; i < KEY_SIZE_FACTOR; ++i)
				{
					//Mark an opaque pixel was found if one of the pixels in the block is not fully transparent.
					if(textureImage.getPixel(x + i, y + j).a != 0)
					{
						hasOpaquePixel = true;
						break;
					}
				}

				//Break out of loop if we have already found an opaque pixel.
				if(hasOpaquePixel) break;
			}

			//Mark the pixel on the destruction key as a collidable pixel, if there was an opaque pixel in this block.
			if(hasOpaquePixel)
			{
				m_keyImage.setPixel(x / KEY_SIZE_FACTOR, y / KEY_SIZE_FACTOR, sf::Color(255, 255, 255));
			}
		}
	}

	//Load key into texture, so it can be used with the fragment shader.
	m_keyTex.loadFromImage(m_keyImage);

	//Load the damage shader from file as a fragment shader.
	m_damageShader.loadFromFile("Assets/damageShader.frag", sf::Shader::Fragment);
	//Set the uniforms for the shader.
	m_damageShader.setUniform("texture", *getTexture());
	m_damageShader.setUniform("keyTexture", m_keyTex);

	//Add turrets to the ship.
	addTurrets(turretList, turretAtlasTexture);
}

//Causes the ship to process internal data to update its state for this tick.
//	deltaTime : The amount of time that has passed since the last update.
void Ship::update(const sf::Time &deltaTime)
{
	//Distance from current position to the target destination.
	sf::Vector2f vectorDistance = m_destination - getPosition();
	//Length of the distance to the destination.
	float vectorLength = sqrt(vectorDistance.x * vectorDistance.x + vectorDistance.y * vectorDistance.y);

	switch(m_movementState)
	{
		//Rotate the ship, and switch to the moving state when we are facing the target.
		case MovementState::ROTATING:
			if(CSB::faceTargetAndCheck(*this, m_destination, deltaTime)) m_movementState = MovementState::MOVING;

			break;
		//Accelerate and move towards target while we are too far to start decelerating.
		case MovementState::MOVING:
			//Increase speed by acceleration.
			m_speed += m_acceleration * deltaTime.asSeconds();
			//Normalise direction and multiply by speed.
			move(vectorDistance / vectorLength * m_speed * deltaTime.asSeconds());

			//Move to deceleration state when deceleration will cause us to stop on the target destination.
			if(vectorLength / m_speed * 2 <= m_speed / m_deceleration) m_movementState = MovementState::DECELERATING;

			break;
		//Declerate until we reach the destination, or we run out of speed.
		case MovementState::DECELERATING:
			//Decrease speed by deceleration.
			m_speed -= m_deceleration * deltaTime.asSeconds();

			//Stop moving; when we run out of speed, or the distance is too small (very small vector lengths can cause infinity).
			if(m_speed <= 0 || vectorLength < 0.0001)
			{
				m_speed = 0;
				m_movementState = MovementState::IDLE;
			}
			//Otherwise, keep moving towards the destination with the ship's remaining speed.
			else
			{
				move(vectorDistance / vectorLength * m_speed * deltaTime.asSeconds());
			}

			break;
	}

	//Lock ship's turret list for processing.
	turretMutex.lock();

	//Process each turret for this tick.
	for(auto &turret : m_turrets)
	{
		turret->update(deltaTime);
	}

	turretMutex.unlock();
}

//Draw the ship, and its children, to the render target.
//	target : What we will draw the ship onto.
//	states : How to manipulate the drawing of the ship.
void Ship::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	//Custom render states that has a custom fragment shader to draw the ship damage.
	sf::RenderStates shipDamageStates = states;

	//Put the shader in the render states.
	shipDamageStates.shader = &m_damageShader;

	//Draw the body of the ship.
	target.draw((sf::Sprite)*this, shipDamageStates);

	//Combine the ship's transform into the render states, so the child objects will move and rotate with it.
	states.transform.combine(getTransform());

	//Lock the turret list for drawing.
	turretMutex.lock();

	//Draw every turret on the ship.
	for(const auto &turret : m_turrets)
	{
		target.draw(*turret, states);
	}

	turretMutex.unlock();
}

//Orders the ship to move to the target position.
//	target : The position to move to.
void Ship::moveCommand(const sf::Vector2f &target)
{
	//We wipe the speed as Ship doesn't support moving while turning.
	m_speed = 0;
	m_destination = target;
	m_movementState = MovementState::ROTATING;
}

//Orders the ship's turrets to fire at the target position.
//	target : The position to fire at.
//	layer : The layer the shot will travel on.
void Ship::fireCommand(const sf::Vector2f &target, unsigned int layer)
{
	turretMutex.lock();

	for(auto &turret : m_turrets)
	{
		turret->fireCommand(target, layer);
	}

	turretMutex.unlock();
}

//Finds if there was a collision between this ship and the passed global position.
//	globalPosition : The global position to check for a collision against.
//Returns whether the collision occurred.
bool Ship::collide(const sf::Vector2f &globalPosition)
{
	//Whether a collision occurred.
	bool didCollide = false;
	//Get position on pixel grid.
	sf::Vector2i pixelPosition = getPixelPosition(globalPosition);

	//Make further collision checks if passed broad-phase pass.
	if(getLocalBounds().contains(sf::Vector2f(pixelPosition)))
	{
		//A collision occured if the pixel at the location is not transparent.
		didCollide = m_keyImage.getPixel(pixelPosition.x / KEY_SIZE_FACTOR, pixelPosition.y / KEY_SIZE_FACTOR).a != 0;
	}

	return didCollide;
}

//Finds if there was a collision between this ship and the passed projectile.
//	proj : Projectile that might have collided with this ship.
//	deltaTime : The amount of time that passed to move the projectile to this position from the last.
//Returns whether the collision occurred.
bool Ship::collide(const Projectile &proj, const sf::Time &deltaTime)
{
	//Whether the projectile collided or not.
	bool didCollide = false;

	//Make further collision checks if the projectile's bounds intersect the ship's bounds.
	if(getGlobalBounds().intersects(proj.getGlobalBounds()))
	{
		//Find the first pixel hit by the projectile.
		sf::Vector2i pixelHit = firstPixelHit(proj, deltaTime);

		//There was a collision if it was not out of bounds.
		if(pixelHit.x != -1)
		{
			//Black out the pixel that was hit on the key.
			m_keyImage.setPixel(pixelHit.x, pixelHit.y, sf::Color(0, 0, 0, 0));
			//Update key with new image.
			m_keyTex.update(m_keyImage);

			//Lock turret list, so we can delete elements safely.
			turretMutex.lock();

			//Erase any turrets that were hit by the projectile.
			for(auto it = m_turrets.begin(); it != m_turrets.end();)
			{
				//Erase the turret if the pixel beneath it no longer exists.
				if(!collide(getTransform().transformPoint((*it)->getPosition())))
				{
					it = m_turrets.erase(it);
				}
				//Otherwise, increment the loop.
				else
				{
					it++;
				}
			}

			turretMutex.unlock();

			didCollide = true;
		}
	}

	return didCollide;
}

//Builds, and adds, turrets made from the build info to this ship.
//	newTurrets : Build information for the new turrets.
//	turretAtlasTexture : Texture atlas to apply to the new turrets.
void Ship::addTurrets(const std::vector<TurretInfo> &newTurrets, const sf::Texture *turretAtlasTexture)
{
	for(const auto &buildInfo : newTurrets)
	{
		m_turrets.push_back(std::make_unique<Turret>(buildInfo, &getTransform(), turretAtlasTexture));
	}
}

//Converts global co-ordinates to the pixel this corresponds to relative to the ship's texture.
//	globalPosition : The global co-ordinates to to transform.
//Returns the pixel co-ordinates that the global co-ordinates transformed to.
sf::Vector2i Ship::getPixelPosition(sf::Vector2f globalPosition) const
{
	return sf::Vector2i(getTransform().getInverse().transformPoint(globalPosition));
}

//Finds the first pixel hit by the projectile.
//	proj : The projectile that might have collided with the ship.
//	deltaTime : The amount of time that passed to move the projectile to this position from the last.
//Returns the first pixel hit, or a value of {-1, -1} if there was no collision.
sf::Vector2i Ship::firstPixelHit(const Projectile &proj, const sf::Time &deltaTime) const
{
	///The following is a tailored implementation of Bresenham's line algorithm to find the first pixel in a line that was hit.
	///I.e. The first pixel that was not transparent.

	//The pixel that was first hit by the projectile; {-1, -1} if a pixel was not hit.
	sf::Vector2i pixelHit = sf::Vector2i(-1, -1);

	//End position of the projectile; where the projectile currently is.
	sf::Vector2f endPosition = proj.getPosition();
	//Start position of the projectile; where the projectile was before it was moved.
	sf::Vector2f startPosition = proj.getPosition() - proj.getVelocity() * deltaTime.asSeconds();

	//Turn the start and end position into pixel co-ordinates, so we can identify the first pixel hit.
	//Divided by KEY_SIZE_FACTOR, as the damage key is usually not the same size as the texture.
	sf::Vector2i pixelStartPosition = sf::Vector2i(sf::Vector2f(getPixelPosition(startPosition)) / static_cast<float>(KEY_SIZE_FACTOR));
	sf::Vector2i pixelEndPosition = sf::Vector2i(sf::Vector2f(getPixelPosition(endPosition)) / static_cast<float>(KEY_SIZE_FACTOR));

	//Whether the line originally had a greater y difference than x difference.
	const bool isSteep = (abs(pixelEndPosition.y - pixelStartPosition.y) > abs(pixelEndPosition.x - pixelStartPosition.x));
	//Swap he x and y co-ordinates if the line is steep; to make the algorithm simpler.
	if(isSteep)
	{
		std::swap(pixelStartPosition.x, pixelStartPosition.y);
		std::swap(pixelEndPosition.x, pixelEndPosition.y);
	}
	//Increase x by -1 each step if the start is greater than the end; otherwise, by 1.
	const int xStep = (pixelStartPosition.x > pixelEndPosition.x) ? -1 : 1;
	//Increase y by -1 each step if the start is greater than the end; otherwise, by 1.
	const int yStep = (pixelStartPosition.y < pixelEndPosition.y) ? 1 : -1;

	//Difference on the x-axis between the start and end.
	const float xDiff = static_cast<float>(abs(pixelEndPosition.x - pixelStartPosition.x));
	//Difference on the y-axis between the start and end.
	const float yDiff = static_cast<float>(abs(pixelEndPosition.y - pixelStartPosition.y));

	//Measures how long until we have to step the y-axis.
	float error = xDiff / 2.0f;
	//Start position on the y-axis.
	int y = pixelStartPosition.y;

	//Step through all x co-ordinates of the line drawn from the start position to the end position..
	for(int x = pixelStartPosition.x; x != pixelEndPosition.x; x += xStep)
	{
		//Only read a pixel from a valid position.
		if(sf::IntRect({0, 0}, (sf::Vector2i)m_keyImage.getSize()).contains(y, x))
		{
			//If the line was originally steep, then we need to swap the x and y co-ordinates back.
			if(isSteep)
			{
				//Mark this as the first pixel hit, and break out of the loop, if this pixel is not transparent.
				if(m_keyImage.getPixel(y, x).a != 0)
				{
					pixelHit = {y, x};
					break;
				}
			}

			//Otherwise, we use the x and y co-ordinates as they are.
			else
			{
				//Mark this as the first pixel hit, and break out of the loop, if this pixel is not transparent.
				if(m_keyImage.getPixel(x, y).a != 0)
				{
					pixelHit = {x, y};
					break;
				}
			}
		}

		//Subtract the y diff from the error to measure how long until we step on the y-axis.
		error -= yDiff;
		//Step on y-axis if the error is less than 0.
		if(error < 0)
		{
			y += yStep;
			//Add xDiff so we can measure when we next need to step on the y-axis.
			error += xDiff;
		}
	}

	return pixelHit;
}