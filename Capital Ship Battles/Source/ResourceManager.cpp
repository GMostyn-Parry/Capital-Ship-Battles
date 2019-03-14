/*
 * Author: George Mostyn-Parry
 */
#include "ResourceManager.hpp"

 //Returns the texture found at the file path; returns nullptr on a failed load.
 //	filePath : File path of the texture we want to load into the resource manager, if it is not already loaded.
sf::Texture* ResourceManager::loadTexture(const std::string &filePath)
{
	//Attempt to load the texture, if the texture does not exist in the table.
	if(m_textureTable.find(filePath) == m_textureTable.end())
	{
		//Texture that will be used to load the image from file.
		sf::Texture loader;

		//Store, and return, the texture if it was successfully loaded.
		if(loader.loadFromFile(filePath))
		{
			m_textureTable[filePath] = loader;
			return &m_textureTable[filePath];
		}
		//Otherwise, return a nullptr.
		else
		{
			return nullptr;
		}
	}
	//Otherwise, return a pointer to the stored texture.
	else
	{
		return &m_textureTable[filePath];
	}
}

//Returns the font found at the file path; returns nullptr on a failed load.
//	filePath : File path of the font we want to load into the resource manager, if it is not already loaded.
sf::Font* ResourceManager::loadFont(const std::string &filePath)
{
	//Attempt to load the font, if the font does not exist in the table.
	if(m_fontTable.find(filePath) == m_fontTable.end())
	{
		//Font class that will be used to load the font from memory.
		sf::Font loader;

		//Store, and return, the font if it was successfully loaded.
		if(loader.loadFromFile(filePath))
		{
			m_fontTable[filePath] = loader;
			return &m_fontTable[filePath];
		}
		//Otherwise, return a nullptr.
		else
		{
			return nullptr;
		}
	}
	//Otherwise, return a pointer to the stored font.
	else
	{
		return &m_fontTable[filePath];
	}
}
