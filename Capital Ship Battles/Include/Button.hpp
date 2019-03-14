/*
 * Author: George Mostyn-Parry
 *
 * A class representing a button used in a GUI.
 * It takes a reference to a function, via std::function, and calls the function when the mouse is released over it.
 */
#pragma once

#include <functional> //For std::function.

#include <SFML/Graphics.hpp> //For SFML graphics.

//A button in a GUI interace; takes a std::function<void()> to perform an action when activated.
class Button : public sf::RectangleShape
{
public:
	//Basic Button constructor.
	//	action : Function called when the button is activated; i.e. from a mouse release event.
	//	globalPosition : Global co-ordinates of where the button appears on the screen.
	Button(const std::function<void()> &action, const sf::Vector2f &globalPosition = sf::Vector2f());
	//Constructor for Button class.
	//	action : Function called when the button is activated; i.e. from a mouse release event.
	//	text : String that will appear on the label of the button.
	//	font : Font used by the button.
	//	globalPosition : Global co-ordinates of where the button appears on the screen.
	Button(const std::function<void()> &action, const std::string &text, const sf::Font &font, const sf::Vector2f &globalPosition = sf::Vector2f());

	//Draws the button onto the RenderTarget.
	//	target : What we will be drawing onto.
	//	states : Visual manipulations to the elements that are being drawn.
	virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const;

	//Checks if the mouse was released over the button; and calls the referenced function if it was.
	//	globalPosition : Global co-ordinates of where the mouse cursor was released.
	//Returns whether the mouse was released on the button.
	bool mouseReleased(const sf::Vector2f &globalPosition);

	//Set the text and font of the button's label.
	//	text : The text the button's label will now display.
	//	font : The font the button's label will now use.
	void setLabel(const std::string &text, const sf::Font &font);
	//Set the text displayed on the label of the button.
	//	text : The text the button's label will now display.
	void setLabelText(const std::string &text);
	//Set the font used by the label of the button.
	//	font : The font the button's label will now use.
	void setLabelFont(const sf::Font &font);
private:
	const std::function<void()> m_action; //The action the button performs on being activated.

	sf::Text m_label; //The text that appears on the button.

	//Centres the label in the button.
	void centreLabel();
};