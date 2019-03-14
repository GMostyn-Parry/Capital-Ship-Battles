/*
 * Author: George Mostyn-Parry
 * Compiled in Visual Studio 2017 with the C++17 standard using SFML 2.5.1.
 * Originally created approximately January 2017, and re-factored and commented in January 2019 - February 2019.
 * Expects SFML 2.5.1 for Visual Studio 2017 in C:\Libraries. This may be changed, simply refer to:
 * https://www.sfml-dev.org/tutorials/2.5/start-vc.php
 *
 * A program to represent a battle; between two identical ships built by the user,
 * or two different ships built by two different people over a network.
 * The user can choose from a selection of three turrets to place on their ship.
 * The battle can be networked between at most two people.
 * Networking and rendering are performed on their own thread.
 * Battle mode has a zoom function; it will keep the mouse cursor over the same global co-ordinate when zooming out,
 * and keep the point zoomed in on in-view, as long as it does not cause the view to leave the view boundaries of the battle;
 * represent by a white ring when fully zoomed out.
 */
#include <memory> //For make_unique.

#include "GameManager.hpp" //State manager controlling execution of the application.
#include "BuildState.hpp" //The starting state.

int main()
{	
	//The manager for the game that will handle the execution of the program.
	GameManager game;
	//Start the game in the build state.
	game.setState(std::make_unique<BuildState>(game));
	//Launch the game, which will end when the window closes.
	game.gameLoop();
}