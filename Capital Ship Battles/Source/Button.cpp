/*
 * Author: George Mostyn-Parry
 */
#include "Button.hpp"

//Basic Button constructor.
//	action : Function called when the button is activated; i.e. from a mouse release event.
//	globalPosition : Global co-ordinates of where the button appears on the screen.
Button::Button(const std::function<void()> &action, const sf::Vector2f &globalPosition)
	:Button(action, "", sf::Font(), globalPosition)
{}

//Constructor for Button class.
//	action : Function called when the button is activated; i.e. from a mouse release event.
//	text : String that will appear on the label of the button.
//	font : Font used by the button.
//	globalPosition : Global co-ordinates of where the button appears on the screen.
Button::Button(const std::function<void()> &action, const std::string &text, const sf::Font &font, const sf::Vector2f &position)
	:m_action(action), m_label(text, font, 25)
{
	//Set up the base RectangleShape class of the button.
	setPosition(position);
	setSize({100, 50});
	setFillColor(sf::Color(100, 100, 100));
	setOutlineThickness(-2.5);
	setOutlineColor(sf::Color(125, 125, 125));

	//Position the label at the centre of the button.
	m_label.setPosition(getSize().x / 2.f, getSize().y / 2.f);
	centreLabel();
}

//Draws the button onto the RenderTarget.
//	target : What we will be drawing onto.
//	states : Visual manipulations to the elements that are being drawn.
void Button::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	//Draw the background.
	target.draw((sf::RectangleShape)*this, states);

	//Combine the backgrounds transform for drawing the label, so the label follows the button.
	states.transform.combine(getTransform());
	//Draw the label.
	target.draw(m_label, states);
}

//Checks if the mouse was released over the button; and calls the referenced function if it was.
//	globalPosition : Global co-ordinates of where the mouse cursor was released.
//Returns whether the mouse was released on the button.
bool Button::mouseReleased(const sf::Vector2f &globalPosition)
{
	//Calls the function, and return true, if the mouse was released over the button.
	if(getGlobalBounds().contains(globalPosition))
	{
		m_action();
		return true;
	}

	//Otherwise, the mouse was not released over the button.
	return false;
}

//Set the text and font of the button's label.
//	text : The text the button's label will now display.
//	font : The font the button's label will now use.
void Button::setLabel(const std::string &text, const sf::Font &font)
{
	m_label.setString(text);
	m_label.setFont(font);

	//Centre label after text change.
	centreLabel();
}

//Set the text displayed on the label of the button.
//	text : The text the button's label will now display.
void Button::setLabelText(const std::string &text)
{
	setLabel(text, *m_label.getFont());
}

//Set the font used by the label of the button.
//	font : The font the button's label will now use.
void Button::setLabelFont(const sf::Font &font)
{
	setLabel(m_label.getString(), font);
}

//Centres the label in the button.
void Button::centreLabel()
{
	m_label.setOrigin(m_label.getGlobalBounds().width / 2.f + m_label.getLineSpacing() * 2, m_label.getGlobalBounds().height);
}