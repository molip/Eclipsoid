#include "Output.h"
#include "App.h"
#include "Model.h"
#include "Team.h"

namespace Output
{

std::string GetRaceName(Race race)
{
	switch (race)
	{
	case Race::Eridani:		return "eridani";
	case Race::Hydran:		return "hydran";
	case Race::Planta:		return "planta";
	case Race::Descendants:	return "descendants";
	case Race::Mechanema:	return "mechanema";
	case Race::Orion:		return "orion";
	case Race::Human:		return "human";
	};
	assert(false);
	return "";
}

std::string GetColourName(Colour colour)
{
	switch (colour)
	{
	case Colour::Black:		return "black";
	case Colour::Blue:		return "blue";
	case Colour::Green:		return "green";
	case Colour::Red:		return "red";
	case Colour::White:		return "white";
	case Colour::Yellow:	return "yellow";
	};
	assert(false);
	return "";
}

const std::string& Message::GetXML() const
{
	ASSERT(!m_pPrinter);
	//Create();
	m_pPrinter.reset(new TiXmlPrinter);
	m_doc.Accept(m_pPrinter.get());
	return m_pPrinter->Str();
}

TiXmlElement* Message::AddElement(const std::string& name, TiXmlNode& parent)
{
	TiXmlElement* pElem = new TiXmlElement(name);
	parent.LinkEndChild(pElem);
	return pElem;
}

//--------------------------------------------

Command::Command(const std::string& cmd) 
{
	m_pRoot = AddElement("command");
	m_pRoot->SetAttribute("type", cmd);
}

Show::Show(const std::string& panel) : Command("show")
{
	m_pRoot->SetAttribute("panel", panel);
}

Update::Update(const std::string& param) : Command("update")
{
	m_pRoot->SetAttribute("param", param);
}

Action::Action(const std::string& param, bool bActive) : Command("action")
{
	m_pRoot->SetAttribute("param", param);
	m_pRoot->SetAttribute("active", bActive);
}

UpdateGameList::UpdateGameList(const Model& model) : Update("game_list")
{
	for (auto& g : model.GetGames())
	{
		auto pGameNode = AddElement("game", *m_pRoot);
		pGameNode->SetAttribute("name", g->GetName());
		pGameNode->SetAttribute("owner", g->GetOwner());
		pGameNode->SetAttribute("started", g->HasStarted());
	
		for (auto& i : g->GetTeams())
			if (i.first != g->GetOwner())
			{
				auto pPlayerNode = AddElement("player", *pGameNode);
				pPlayerNode->SetAttribute("name", i.first);
			}
	}
}

UpdateLobby::UpdateLobby(const Game& game) : Update("lobby")
{
	m_pRoot->SetAttribute("owner", game.GetOwner());
	m_pRoot->SetAttribute("game", game.GetName());
	for (auto& i : game.GetTeams())
		if (i.first != game.GetOwner())
		{
			auto pPlayerNode = AddElement("player", *m_pRoot);
			pPlayerNode->SetAttribute("name", i.first);
		}
}

UpdateLobbyControls::UpdateLobbyControls(bool bShow) : Update("lobby_controls")
{
	m_pRoot->SetAttribute("show", bShow);
}

UpdateChoose::UpdateChoose(const Game& game) : Update("choose_team")
{
	m_pRoot->SetAttribute("game", game.GetName());
	for (auto& i : game.GetTeamOrder())
	{
		auto pTeamNode = AddElement("team", *m_pRoot);
		pTeamNode->SetAttribute("name", i);
		if (const Team* pTeam = game.GetTeam(i)) // Otherwise not chosen yet.
		{
			pTeamNode->SetAttribute("race", GetRaceName(pTeam->GetRace()));
			pTeamNode->SetAttribute("colour", GetColourName(pTeam->GetColour()));
		}
	}
}

UpdateGame::UpdateGame(const Game& game) : Update("game")
{
}

ShowGameList::ShowGameList() :	Show("game_list_panel") {}
ShowChoose::ShowChoose() :		Show("choose_panel") {}
ShowGame::ShowGame() :			Show("game_panel") {}
ShowLobby::ShowLobby() :		Show("lobby_panel") {}

ActionChoose::ActionChoose(const Game& game, bool bActive) : Action("choose_team", bActive)
{
	if (!bActive)
		return;

	auto pRacesNode = AddElement("races", *m_pRoot);
	for (int i = 0; i < (int)Race::_Count; ++i)
		AddElement("race", *pRacesNode)->SetAttribute("name", GetRaceName(Race(i)));

	auto pColoursNode = AddElement("colours", *m_pRoot);
	for (int i = 0; i < (int)Colour::_Count; ++i)
		AddElement("colour", *pColoursNode)->SetAttribute("name", GetColourName(Colour(i)));
}

} // namespace


