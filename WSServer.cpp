#include "WSServer.h"
#include "App.h"
#include "Util.h"
#include "Controller.h"

WSServer::WSServer(Controller& controller) : MongooseServer(8998), m_controller(controller)
{
	controller.SetServer(this);
}

void WSServer::OnConnect(int port, const std::string& url)
{
	std::cout << "INFO: Port connected: " << port << std::endl;
}

void WSServer::OnMessage(int port, const std::string& message)
{
	auto v = Util::SplitString(message, ':');
	if (v.empty())
		return;
	
	std::string cmd = v[0];
	v.erase(v.begin());

	if (cmd == "REGISTER")
	{
		if (v.size() == 1)
			RegisterPlayer(port, v[0]);
		else
			__super::SendMessage(port, "ERROR:NO_PLAYER");
	}
	else
	{
		auto i = m_mapPortToPlayer.find(port);
		if (i != m_mapPortToPlayer.end())
			m_controller.OnCommand(i->second, cmd, v);
	}
}

void WSServer::RegisterPlayer(int port, const std::string& player)
{
	LOCK(m_mutex);

	auto i = m_mapPlayerToPort.find(player);
	if (i != m_mapPlayerToPort.end())
	{
		__super::SendMessage(port, "ERROR:DUPLICATE_PLAYER");
		return;
	}
		
	m_mapPlayerToPort[player] = port;
	m_mapPortToPlayer[port] = player;

	std::cout << "INFO: Port registered: " << port << " -> " << player << std::endl;
	__super::SendMessage(port, "SHOW:PANEL_GAMELIST");
	m_controller.UpdateGameList(player);
}

void WSServer::OnDisconnect(int port) 
{
	LOCK(m_mutex);

	auto i = m_mapPortToPlayer.find(port);
	if (i == m_mapPortToPlayer.end())
		std::cerr << "ERROR: Unregistered port disconnected: " << port << std::endl;
	else
	{
		std::cout << "INFO: Port disconnected: " << port << std::endl;
		m_mapPlayerToPort.erase(i->second);
		m_mapPortToPlayer.erase(port);
	}
}

bool WSServer::SendMessage(const std::string& msg, const std::string& player) const
{
	if (player.empty())
		BroadcastMessage(msg);
	else
	{
		auto i = m_mapPlayerToPort.find(player);
		if (i == m_mapPlayerToPort.end())
			return false;

		__super::SendMessage(i->second, msg);
	}
	return true;
}

void WSServer::BroadcastMessage(const std::string& msg) const
{
	for (auto i : m_mapPortToPlayer)
		__super::SendMessage(i.first, msg);
}