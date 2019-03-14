/*
 * Author: George Mostyn-Parry
 *
 * A simple class to manage the access, and lifetime, of resources; such as textures, and fonts.
 */
#pragma once

#include <SFML/Graphics.hpp> //For sf::Texture.

//A simple class to manage the access, and lifetime, of resources; such as textures, and fonts.
class ResourceManager
{
public:
	//Returns the texture found at the file path; returns nullptr on a failed load.
	//	filePath : File path of the texture we want to load into the resource manager, if it is not already loaded.
	sf::Texture* loadTexture(const std::string &filePath);
	//Returns the font found at the file path; returns nullptr on a failed load.
	//	filePath : File path of the font we want to load into the resource manager, if it is not already loaded.
	sf::Font* loadFont(const std::string &filePath);
private:
	std::map<std::string, sf::Texture> m_textureTable; //Table of all of the texures stored in the manager.
	std::map<std::string, sf::Font> m_fontTable; //Table of all of the fonts stored in the manager.
};

