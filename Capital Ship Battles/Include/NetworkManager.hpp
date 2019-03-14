/*
 * Author: George Mostyn-Parry
 *
 * Networks battles between two players.
 * The players need to be connected together before the battle starts, and the BattleState told to use networking.
 * Uses TCP sockets; the packets are not that regular, as we only send the commands and not constant updates of state.
 * 
 * This class does not guarantee the battle will remain in sync. It only ensures all commands are sent between the two players.
 */
#pragma once

#include <SFML/Network.hpp> //For networking with SFML.

#include "Turret.hpp" //Needed for storing turret build info.

class BattleState; //Declaration of BattleState for declaration of NetworkManager.

//What type of packet is being sent or received.
enum class PacketType : uint8_t
{
	CONNECT,
	DISCONNECT,
	MOVE,
	FIRE
};

//Class for connecting two players together, networking a battle between them,
//and handles the receiving and sending of packets over a TCPSocket.
class NetworkManager
{
public:
	//Listens for a client attempting to join on the local user.
	//Returns whether a server was successfully set up.
	bool hostServer();
	//Attempt to join a server on the passed IP; non-blocking so it may be interrupted.
	//	rawIP : IP of the server we are attempting to join, as a string.
	//Returns whether the join attempt was successful.
	bool joinServer(const std::string &IP);

	//Stops any connections, and any attempts to connect to another user.
	void closeAllConnections();

	//Handles receiving of packets from the peer; the main loop of the network manager.
	void receive();
	//Send a packet to the other user.
	// packet : The network packet that will be sent.
	void send(sf::Packet packet);

	//Sets the turret list of the ship built by the local player.
	//	shipTurrets : List of information to build the turrets on the local player's ship.
	void setTurretList(const std::vector<TurretInfo> &shipTurrets);
	//Sets the battle to the passed value.
	//	newBattle : The battle we want the network manager to handle the networking for.
	void setBattle(BattleState *newBattle);
	//Sends the ship built by the local user to the other user.
	void sendShip();

	//Returns whether the local player is the host.
	bool isHost() const;
private:
	static constexpr unsigned int PORT = 25565; //The port the server is being run on.

	bool m_isHost = false; //Whether the local player is the host.

	sf::TcpSocket m_socket; //Socket that manages the connection to the other player.
	sf::TcpListener m_listener; //Listener for gaining new clients.

	std::vector<TurretInfo> m_shipTurrets; //List of turrets that the local user placed on their ship.

	BattleState *m_battle; //The battle that is being networked.		
	
	//Unpackages the data of the ship built by the other user, and creates the ship in the battle.
	//	shipPacket : Network packet containing the data on the ship.
	void unpackageShip(sf::Packet shipPacket);
};

//Returns whether the local player is the host.
inline bool NetworkManager::isHost() const
{
	return m_isHost;
}