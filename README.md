Author: George Mostyn-Parry\
Compiled in Visual Studio 2017 with the C++17 standard using SFML 2.5.1.\
Originally created approximately January 2017, and re-factored and commented in January 2019 - February 2019.\
To compile it expects SFML 2.5.1 for Visual Studio 2017 in C:\Libraries. This may be changed, simply refer to:\
https://www.sfml-dev.org/tutorials/2.5/start-vc.php

A program to represent a battle; between two identical ships built by the user, or two different ships built by two different people over a network.\
The user can choose from a selection of three turrets to place on their ship.\
The battle can be networked between at most two people.\
Networking and rendering are performed on their own thread.\
Battle mode has a zoom function; it will keep the mouse cursor over the same global co-ordinate when zooming out, and keep the point zoomed in on in-view, as long as it does not cause the view to leave the view boundaries of the battle; represented by a white ring when fully zoomed out.
#Demonstration Video
https://youtu.be/CV7uR4MWoqg

# USAGE
You can close the window at any time by clicking the titlebar close button on the window, or by using the 'Escape' key.
Pressing Alt+Enter will toggle the window between fullscreen.

To host a server you must port forward on port 25565 for TCP.

## Build State
When the game starts you will be placed in the build state, here you can:
- Attach turrets to a ship.
- Go direct to a singleplayer battle against an identical ship.
- Move to the connect state.

Clicking the buttons in the top-left will change what turret you are currently placing.\
Pressing the key 1-3 will also change the current turret.\
Left-clicking on the hull will attach the turret to the ship.\
Right-clicking will cause any turrets obstructing the preview turret to be removed.\
F3 will build a "debug" ship; this ship is essentially a ship with the maximum amount of turrets (4x4).\

Clicking the "Multi" button will bring you to the connect state.\
Clicking the "Local" button will bring you to the battle state in local mode.\

## Connect State
At the start of the state you will be presented with two buttons:
- The "Host" button will start hosting a server on port 25565.
- The "Join" button will take you to a screen where you can enter the IP to join on.

In the join micro-state you may:
- Type to input the IP address you want to join on.
- Use backspace to remove characters.
- Press the return key to attempt to join.
- Click the "Join" button to attempt to join.
Any join attempt will timeout after five seconds; after which you will informed of the failure and may re-enter the IP.\
Entering nothing, or "localhost", will cause the player to join on their own IP.\

At any time you may leave the Connect State, and go back to the Build State, by clicking the "Leave" button in the top-left.

## Battle State
In the battle state you will be presented with two ships; either of the same design, or different design, depending on if it is a local match.
- Left-clicking will fire all of the ship's turrets at the mouse position.
- Right-clicking will move the ship to the mouse position.
- The mouse-wheel will zoom the view in and out.

The only way to move around the battlefield is to zoom out, then zoom in.

Any hostile projectile that hits a ship will cause it to lose a pixel in the place it was hit; a turret is removed if the pixel it is place on is removed.\
The battle ends when either ship loses all of their turrets.\
The player will be brought back to the Build State with their turret configuration placed on the hull.
