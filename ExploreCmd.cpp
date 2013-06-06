#include "stdafx.h"
#include "ExploreCmd.h"
#include "Output.h"
#include "Input.h"
#include "Controller.h"
#include "Player.h"
#include "Race.h"
#include "LiveGame.h"
#include "Map.h"
#include "DiscoverCmd.h"
#include "Record.h"
#include "EdgeSet.h"

ExploreCmd::ExploreCmd(Player& player, int iPhase) : Cmd(player), m_idHex(-1), m_iPos(-1), m_iPhase(iPhase)
{
	std::set<MapPos> positions;
	const Map::HexMap& hexes = GetGame().GetMap().GetHexes();
	for (auto& h : hexes)
		if (h.second->IsOwnedBy(GetTeam())) // TODO: Check ships.
			GetGame().GetMap().GetEmptyNeighbours(h.first, GetTeam().HasTech(TechType::WormholeGen), positions);

	m_positions.insert(m_positions.begin(), positions.begin(), positions.end());
}

void ExploreCmd::UpdateClient(const Controller& controller) const
{
	controller.SendMessage(Output::ChooseExplorePos(m_positions, m_iPhase > 1), m_player);
}

CmdPtr ExploreCmd::Process(const Input::CmdMessage& msg, const Controller& controller)
{
	if (dynamic_cast<const Input::CmdExploreReject*>(&msg))
		return nullptr;

	auto& m = CastThrow<const Input::CmdExplorePos>(msg);
	AssertThrow("ExploreCmd::Process: invalid pos index", InRange(m_positions, m.m_iPos));
	m_iPos = m.m_iPos;

	const MapPos& pos = m_positions[m_iPos];

	m_idHex = GetGame().GetHexBag(pos.GetRing()).TakeTile();
	
	std::vector<int> hexIDs;
	hexIDs.push_back(m_idHex);
	return CmdPtr(new ExploreHexCmd(m_player, pos, hexIDs, m_iPhase));
}

//-----------------------------------------------------------------------------

class ExploreRecord : public Record
{
public:
	ExploreRecord(Colour colour, const MapPos& pos, int idHex, int iRot, bool bInfluence) : 
		m_colour(colour), m_pos(pos), m_idHex(idHex), m_iRot(iRot), m_bInfluence(bInfluence), m_discovery(DiscoveryType::None) {}

	DiscoveryType GetDiscovery() const { return m_discovery; }

	virtual void Do(Game& game, const Controller& controller) override
	{
		Hex& hex = game.GetMap().AddHex(m_pos, m_idHex, m_iRot);
				
		if (m_bInfluence)
		{
			hex.SetColour(m_colour);

			Team& team = game.GetTeam(m_colour);
			team.GetInfluenceTrack().RemoveDiscs(1);
			controller.SendMessage(Output::UpdateInfluenceTrack(team), game);
			
			m_discovery = hex.GetDiscoveryTile();
			if (m_discovery != DiscoveryType::None)
				hex.RemoveDiscoveryTile();

		}					
		controller.SendMessage(Output::UpdateMap(game), game);
	}

	virtual void Undo(Game& game, const Controller& controller) override
	{
		if (m_bInfluence)
		{
			Team& team = game.GetTeam(m_colour);
			team.GetInfluenceTrack().AddDiscs(1);
			controller.SendMessage(Output::UpdateInfluenceTrack(team), game);

			if (m_discovery != DiscoveryType::None)
				game.GetMap().GetHex(m_pos).SetDiscoveryTile(m_discovery);
		}					
		game.GetMap().DeleteHex(m_pos);
		controller.SendMessage(Output::UpdateMap(game), game);
	}

private:
	Colour m_colour;
	MapPos m_pos;
	int m_idHex, m_iRot;
	bool m_bInfluence;
	DiscoveryType m_discovery;
};

//-----------------------------------------------------------------------------

ExploreHexCmd::ExploreHexCmd(Player& player, const MapPos& pos, std::vector<int> hexIDs, int iPhase) : 
	Cmd(player), m_pos(pos), m_iPhase(iPhase), m_hexIDs(hexIDs), m_iRot(-1), m_iHex(-1), m_bInfluence(false), 
	m_idTaken(-1), m_discovery(DiscoveryType::None)
{
	bool bWormholeGen = GetTeam().HasTech(TechType::WormholeGen);

	std::vector<const Hex*> hexes = GetGame().GetMap().GetSurroundingHexes(m_pos, GetTeam());

	Race race(GetTeam().GetRace());

	m_hexChoices.clear();
	
	for (int id : hexIDs)
	{
		Hex hex(id, pos, 0);

		bool bCanInfluenceHex = hex.GetShips().empty(); // Any ships must be ancients.
		HexChoice hc(id, bCanInfluenceHex && GetTeam().GetInfluenceTrack().GetDiscCount() > 0);
		
		EdgeSet inner = hex.GetWormholes();

		for (int i = 0; i < 6; ++i) // Try each rotation.
		{
			bool bMatch = false;
			for (auto e : EnumRange<Edge>())
			{
				const Hex* pHex = hexes[(int)e];
				
				bool bInner = inner[RotateEdge(e, -i)];
				bool bOuter = pHex && pHex->HasWormhole(ReverseEdge(e));

				if (bWormholeGen)
					bMatch = pHex && (bOuter || bInner);
				else
					bMatch = bOuter && bInner;

				if (bMatch)
					break;
			}
			if (bMatch)
				hc.m_rotations.push_back(i);
		}
		m_hexChoices.push_back(hc);
	}
}

void ExploreHexCmd::UpdateClient(const Controller& controller) const
{
	bool bCanTake = Race(GetTeam().GetRace()).GetExploreChoices() > (int)m_hexChoices.size();

	Output::ChooseExploreHex msg(m_pos.GetX(), m_pos.GetY(), bCanTake, GetGame().CanRemoveCmd());
	for (auto& hc : m_hexChoices)
		msg.AddHexChoice(hc.m_idHex, hc.m_rotations, hc.m_bCanInfluence);

	controller.SendMessage(msg, m_player);
}


CmdPtr ExploreHexCmd::Process(const Input::CmdMessage& msg, const Controller& controller)
{
	Game& game = GetGame();

	m_discovery = DiscoveryType::None;

	if (dynamic_cast<const Input::CmdExploreHexTake*>(&msg))
	{
		m_idTaken = GetGame().GetHexBag(m_pos.GetRing()).TakeTile(); 
		std::vector<int> hexIDs = m_hexIDs;
		hexIDs.insert(hexIDs.begin(), m_idTaken); // At front so it appears first.
		m_idTaken = true;
		return CmdPtr(new ExploreHexCmd(m_player, m_pos, hexIDs, m_iPhase));
	}

	bool bReject = !!dynamic_cast<const Input::CmdExploreReject*>(&msg);
	if (bReject)
	{
		// TODO: Add discarded hexes to discard pile. 
	}
	else
	{
		auto& m = CastThrow<const Input::CmdExploreHex>(msg);

		AssertThrowXML("ExploreHexCmd::AcceptMessage", InRange(m_hexChoices, m.m_iHex));
		m_iHex = m.m_iHex;

		auto& hc = m_hexChoices[m_iHex];
	
		AssertThrowXML("ExploreHexCmd: rotation index", InRange(hc.m_rotations, m.m_iRot));
		m_iRot = m.m_iRot;

		ExploreRecord* pRec = new ExploreRecord(GetTeam().GetColour(), m_pos, m_hexChoices[m_iHex].m_idHex, hc.m_rotations[m_iRot], m.m_bInfluence);
		pRec->Do(GetGame(), controller);
		m_discovery = pRec->GetDiscovery();
		GetGame().PushRecord(RecordPtr(pRec));
	}

	bool bFinished = m_iPhase + 1 == Race(GetTeam().GetRace()).GetExploreRate();
	Cmd* pNext = bFinished ? nullptr : new ExploreCmd(m_player, m_iPhase + 1);
	
	if (m_discovery != DiscoveryType::None)
		pNext = new DiscoverCmd(m_player, m_discovery, CmdPtr(pNext));

	return CmdPtr(pNext);
}

void ExploreHexCmd::Undo(const Controller& controller)
{
	if (!HasRecord()) // Rejected
		return;

	RecordPtr pRec = GetGame().PopRecord();
	pRec->Undo(GetGame(), controller);
}
