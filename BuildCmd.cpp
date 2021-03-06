#include "stdafx.h"
#include "BuildCmd.h"
#include "Output.h"
#include "Input.h"
#include "Controller.h"
#include "LiveGame.h"
#include "Record.h"
#include "CommitSession.h"

namespace
{
	Buildable ShipToBuildable(ShipType s)
	{
		switch (s)
		{
		case ShipType::Interceptor:	return Buildable::Interceptor;
		case ShipType::Cruiser:		return Buildable::Cruiser;
		case ShipType::Dreadnought:	return Buildable::Dreadnought;
		case ShipType::Starbase:	return Buildable::Starbase;
		}
		VERIFY_MODEL(false);
		return Buildable::None;
	}

	ShipType BuildableToShip(Buildable b)
	{
		switch (b)
		{
		case Buildable::Interceptor:	return ShipType::Interceptor;
		case Buildable::Cruiser:		return ShipType::Cruiser;
		case Buildable::Dreadnought:	return ShipType::Dreadnought;
		case Buildable::Starbase:		return ShipType::Starbase;
		}
		VERIFY_MODEL(false);
		return ShipType::None;
	}

	bool CanBuild(const Team& team, Buildable b)
	{
		if (b != Buildable::Orbital && b != Buildable::Monolith && team.GetUnusedShips(BuildableToShip(b)) < 1)
			return false;

		if (b == Buildable::Starbase && !team.HasTech(TechType::StarBase) ||
			b == Buildable::Orbital && !team.HasTech(TechType::Orbital) ||
			b == Buildable::Monolith && !team.HasTech(TechType::Monolith))
			return false;

		return Race(team.GetRace()).GetBuildCost(b) <= team.GetStorage()[Resource::Materials];
	}
}

//-----------------------------------------------------------------------------

class BuildRecord : public TeamRecord
{
public:
	BuildRecord() : m_buildable(Buildable::None), m_idHex(0){}
	BuildRecord(Colour colour, const MapPos& pos, Buildable buildable) : TeamRecord(colour), m_pos(pos), m_buildable(buildable), m_idHex(0){}

	virtual void Save(Serial::SaveNode& node) const override 
	{
		__super::Save(node);
		node.SaveEnum("buildable", m_buildable);
		node.SaveType("pos", m_pos);
		node.SaveType("hex_id", m_idHex);
	}
	
	virtual void Load(const Serial::LoadNode& node) override 
	{
		__super::Load(node);
		node.LoadEnum("buildable", m_buildable);
		node.LoadType("pos", m_pos);
		node.LoadType("hex_id", m_idHex);
	}

private:
	virtual void Apply(bool bDo, const Game& game, const Team& team, GameState& gameState, TeamState& teamState) override
	{
		Hex& hex = gameState.GetMap().GetHex(m_pos);
		m_idHex = hex.GetID();

		if (m_buildable == Buildable::Orbital)
			hex.SetOrbital(bDo);
		else if (m_buildable == Buildable::Monolith)
			hex.SetMonolith(bDo);
		else
		{
			ShipType s = BuildableToShip(m_buildable);
			if (bDo)
			{
				hex.AddShip(s, m_colour);
			}
			else
			{
				hex.RemoveShip(s, m_colour);
			}
			teamState.AddShips(s, bDo ? -1 : 1);
		}

		int cost = Race(team.GetRace()).GetBuildCost(m_buildable);

		teamState.GetStorage()[Resource::Materials] += bDo ? -cost : cost;
	}

	virtual void Update(const Game& game, const Team& team, const RecordContext& context) const override
	{
		context.SendMessage(Output::UpdateMap(game));
		context.SendMessage(Output::UpdateStorageTrack(team));
	}

	virtual std::string GetTeamMessage() const 
	{
		return FormatString("built %0 in hex %1", ::EnumToString(m_buildable), m_idHex);
	}

	Buildable m_buildable;
	MapPos m_pos;
	int m_idHex;
};

REGISTER_DYNAMIC(BuildRecord)

//-----------------------------------------------------------------------------

BuildCmd::BuildCmd(Colour colour, const LiveGame& game, int iPhase) : PhaseCmd(colour, iPhase)
{
}

void BuildCmd::UpdateClient(const Controller& controller, const LiveGame& game) const
{
	const Team& team = GetTeam(game);

	std::set<ShipType> ships;
	for (auto s : PlayerShipTypesRange())
		if (CanBuild(team, ShipToBuildable(s)))
			ships.insert(s);

	std::map<MapPos, std::pair<bool, bool>> hexes; // (orbital, monolith)

	for (auto& h : game.GetMap().GetHexes()) // Pos, hex.
		if (h.second->GetColour() == m_colour)
			hexes[h.first] = std::make_pair(!h.second->HasOrbital() && CanBuild(team, Buildable::Orbital),
											!h.second->HasMonolith() && CanBuild(team, Buildable::Monolith));

	controller.SendMessage(Output::ChooseBuild(ships, hexes, m_iPhase > 0), GetPlayer(game));
}

Cmd::ProcessResult BuildCmd::Process(const Input::CmdMessage& msg, CommitSession& session)
{
	if (dynamic_cast<const Input::CmdAbort*>(&msg))
	{
		VERIFY_INPUT(m_iPhase > 0);
		return nullptr;
	}

	auto& m = VerifyCastInput<const Input::CmdBuild>(msg);
	MapPos pos(m.m_x, m.m_y);
	bool bOrbital = m.m_buildable == Buildable::Orbital;
	bool bMonolith = m.m_buildable == Buildable::Monolith;

	const LiveGame& game = session.GetGame();
	const Team& team = GetTeam(game);
	const Hex& hex = game.GetMap().GetHex(pos);
	VERIFY_INPUT_MSG("invalid hex", hex.GetColour() == m_colour);
	VERIFY_INPUT_MSG("team can't build this", CanBuild(team, m.m_buildable));
	VERIFY_INPUT_MSG("already got orbital", !bOrbital || !hex.HasOrbital());
	VERIFY_INPUT_MSG("already got monolith", !bMonolith || !hex.HasMonolith());

	BuildRecord* pRec = new BuildRecord(m_colour, pos, m.m_buildable);
	DoRecord(RecordPtr(pRec), session);

	if (!team.HasPassed() && m_iPhase + 1 < Race(team.GetRace()).GetBuildRate())
		return ProcessResult(new BuildCmd(m_colour, game, m_iPhase + 1));

	return nullptr;
}

REGISTER_DYNAMIC(BuildCmd)
