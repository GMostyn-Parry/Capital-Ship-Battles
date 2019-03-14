/*
 * Author: George Mostyn-Parry
 */
#include "NetworkManager.hpp"

#include <BattleState.hpp> //For sending information to the battle we are networking.

//Listens for a client attempting to join on the local user.
//Returns whether a server was successfully set up.
bool NetworkManager::hostServer()
{
	m_isHost = true;

	//Listen on the defined port.
	m_listener.listen(PORT);

	//Accept the next client to connect to the server.
	//If the socket is "Done", then it managed to gain a client.
	return m_listener.accept(m_socket) == sf::Socket::Done;
}

//Attempt to join a server on the passed IP; non-blocking so it may be interrupted.
//	rawIP : IP of the server we are attempting to join, as a string.
//Returns whether the join attempt was successful.
bool NetworkManager::joinServer(const std::string &rawIP)
{
	m_isHost = false;

	//Attempt to connect to the passed IP; with a five second time-out.
	return m_socket.connect(sf::IpAddress(rawIP), PORT, sf::seconds(5)) == sf::Socket::Done;
}

//Stops any connections, and any attempts to connect to another user.
void NetworkManager::closeAllConnections()
{
	//Stops any connection attempts.
	m_listener.close();
	//Close current connection.
	m_socket.disconnect();
}

//Handles receiving of packets from the peer; the main loop of the network manager.
void NetworkManager::receive()
{
	//Holds data of most recently received packet.
	sf::Packet packet;
	//Listen for and handle packets, while they are still being sent.
	while(m_socket.receive(packet) == sf::Socket::Done)
	{
		//Packet type as raw value, as sf::Packet can not store enum classes.
		std::underlying_type_t<PacketType> rawPacketType;
		//Identifying number of the ship.
		unsigned int shipID;
		//Global position of the information; i.e. where to move to, or where to shoot at.
		sf::Vector2f globalPosition;

		//Unpackage the packet type from the packet.
		packet >> rawPacketType;

		//The type of packet we received.
		PacketType receivedType = static_cast<PacketType>(rawPacketType);

		switch(receivedType)
		{
			//Unpackage the ship we received if it was a connection packet.
			case PacketType::CONNECT:
				unpackageShip(packet);

				break;
			//Disconnect from the server, if the peer disconnected.
			case PacketType::DISCONNECT:
				m_socket.disconnect();

				break;
			//Move the enemy ship.
			case PacketType::MOVE:
				packet >> shipID;
				packet >> globalPosition.x;
				packet >> globalPosition.y;
				m_battle->issueMoveCommand(1, shipID, globalPosition);

				break;
			//Order the enemy ship to fire on the target.
			case PacketType::FIRE:
				packet >> shipID;
				packet >> globalPosition.x;
				packet >> globalPosition.y;
				m_battle->issueFireCommand(1, shipID, globalPosition, 0);

				break;
		}
	}
}

//Send a packet to the other user.
void NetworkManager::send(sf::Packet packet)
{
	m_socket.send(packet);
}

//Sets the turret list of the ship built by the local player.
//	shipTurrets : List of information to build the turrets on the local player's ship.
void NetworkManager::setTurretList(const std::vector<TurretInfo>& shipTurrets)
{
	m_shipTurrets = shipTurrets;
}

//Sets the battle to the passed value.
//	newBattle : The battle we want the network manager to handle the networking for.
void NetworkManager::setBattle(BattleState *newBattle)
{
	m_battle = newBattle;
}

//Sends the ship built by the local user to the other user.
void NetworkManager::sendShip()
{
	//Create a connection packet, which will store the constructed ship's information.
	sf::Packet packet;
	packet << std::underlying_type_t<PacketType>(PacketType::CONNECT);

	//Place the host's ship in the top-left.
	if(m_isHost)
	{
		//Package the ship's position.
		packet << 1800.f;
		packet << 1800.f;
		//Package the ship's angle.
		packet << 45.f;
	}
	//Otherwise, place the joining user's ship in the bottom-right.
	else
	{
		//Package the ship's position.
		packet << 2200.f;
		packet << 2200.f;
		//Package the ship's angle.
		packet << 225.f;
	}

	//Package how many turrets the ship has.
	packet << m_shipTurrets.size();

	//Package all of the info for building the turrets.
	for(const auto &turretInfo : m_shipTurrets)
	{
		//Package what type of projectile the turret shoots.
		packet << std::underlying_type_t<ProjectileType>(turretInfo.projType);
		//Package the position of the turret on the ship.
		packet << turretInfo.localPosition.x;
		packet << turretInfo.localPosition.y;
	}

	//Send the information on the ship to the peer.
	send(packet);
}

//Unpackages the data of the ship built by the other user, and creates the ship in the battle.
	//	shipPacket : Network packet containing the data on the ship.
void NetworkManager::unpackageShip(sf::Packet shipPacket)
{
	//The position of the received ship.
	sf::Vector2f shipPosition;
	shipPacket >> shipPosition.x; shipPacket >> shipPosition.y;

	//The ship's angle.
	float angle;
	shipPacket >> angle;

	//How many turrets the ship has.
	size_t turretAmount;
	shipPacket >> turretAmount;

	//List of build information of the turrets on the ship.
	std::vector<TurretInfo> turretList;

	//Unpackage the information of the turrets on the ship.
	for(unsigned int i = 0; i < turretAmount; i++)
	{
		//The projectile the turret fires.
		std::underlying_type_t<ProjectileType> projType;
		shipPacket >> projType;

		//Position of the turret.
		sf::Vector2f position;
		shipPacket >> position.x;
		shipPacket >> position.y;
		
		//Add the unpackaged information to the list.
		turretList.push_back({static_cast<ProjectileType>(projType), position});
	}

	//Create the received ship.
	m_battle->createShip(1, shipPosition, angle, turretList);
}
