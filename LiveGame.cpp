#include "stdafx.h"
#include "LiveGame.h"
#include "App.h"
#include "Player.h"
#include "CmdStack.h"
#include "EnumTraits.h"
#include "Players.h"
#include "Record.h"
#include "Serial.h"
#include "ActionPhase.h"
#include "ChooseTeamPhase.h"
#include "UpkeepPhase.h"

#include <algorithm>

LiveGame::LiveGame() : m_gamePhase(GamePhase::Lobby), m_nextRecordID(1)
{
}

LiveGame::LiveGame(int id, const std::string& name, const Player& owner) : 
Game(id, name, owner), m_gamePhase(GamePhase::Lobby), m_nextRecordID(1)
{
}

LiveGame::~LiveGame()
{
}

void LiveGame::AddPlayer(const Player& player)
{
	VerifyModel("LiveGame::AddTeam: Game already started: " + m_name, !HasStarted());
	if (!FindTeam(player))
		m_teams.push_back(TeamPtr(new Team(player.GetID())));
}

void LiveGame::StartChooseTeamGamePhase()
{
	VerifyModel("LiveGame::Start: Game already started: " + m_name, !HasStarted());
	VerifyModel("LiveGame::Start: Game has no players: " + m_name, !m_teams.empty());
	
	m_gamePhase = GamePhase::ChooseTeam;
	m_pPhase = PhasePtr(new ChooseTeamPhase(this));

	// Decide team order.
	std::shuffle(m_teams.begin(), m_teams.end(), GetRandom());

	//for (int i = 0; i < FakePlayers; ++i)
	//{
	//	std::ostringstream ss;
	//	ss << "Fake " << i;
	//	m_teams.insert(std::make_pair(ss.str(), nullptr));
	//	m_teamOrder.insert(m_teamOrder.begin() + i, ss.str());
	//	AssignTeam(ss.str(), RaceType::Human, Colour(i + 1));
	//}

	// Initialise hex bags.
	for (auto r : EnumRange<HexRing>())
		m_hexBag[(int)r] = HexBag(r, m_teams.size());

	//m_hexBag[1].ReturnTile(206); // Discovery, no ancients, 1 materials square.

	m_repBag.Init();
	m_techBag.Init();
	m_discBag.Init();
}

void LiveGame::StartMainGamePhase()
{
	VerifyModel("LiveGame::StartMainGamePhase", m_gamePhase == GamePhase::ChooseTeam);

	m_gamePhase = GamePhase::Main;

	Hex& centre = m_map.AddHex(MapPos(0, 0), 001, 0);
	centre.AddShip(ShipType::GCDS, Colour::None);

	// Initialise starting hexes.
	auto startPositions = m_map.GetTeamStartPositions();
	assert(startPositions.size() == m_teams.size());
	for (size_t i = 0; i < m_teams.size(); ++i)
	{
		Team& team = *m_teams[i];
		Race r(team.GetRace());
		int idHex = r.GetStartSector(team.GetColour());

		Hex& hex = m_map.AddHex(startPositions[i], idHex, 0);

		team.PopulateStartHex(hex);
	}
	StartActionPhase();
}

Phase& LiveGame::GetPhase()
{
	VerifyModel("LiveGame::GetPhase", !!m_pPhase);
	return *m_pPhase;
}

ActionPhase& LiveGame::GetActionPhase()
{
	VerifyModel("LiveGame::GetActionPhase", m_pPhase && dynamic_cast<ActionPhase*>(m_pPhase.get()));
	return static_cast<ActionPhase&>(*m_pPhase);
}

ChooseTeamPhase& LiveGame::GetChooseTeamPhase()
{
	VerifyModel("LiveGame::GetChooseTeamPhase", m_pPhase && dynamic_cast<ChooseTeamPhase*>(m_pPhase.get()));
	return static_cast<ChooseTeamPhase&>(*m_pPhase);
}

UpkeepPhase& LiveGame::GetUpkeepPhase()
{
	VerifyModel("LiveGame::GetUpkeepPhase", m_pPhase && dynamic_cast<UpkeepPhase*>(m_pPhase.get()));
	return static_cast<UpkeepPhase&>(*m_pPhase);
}

bool LiveGame::NeedCombat() const
{
	return false;
}

void LiveGame::StartActionPhase()
{
	m_pPhase = PhasePtr(new ActionPhase(this));
}

void LiveGame::FinishActionPhase(const std::vector<Colour>& passOrder, const HexArrivals& hexArrivalOrder)
{
	VerifyModel("LiveGame::FinishActionPhase", passOrder.size() == m_teams.size());
	
#if 0	// Special passing rule.
	std::map<Colour, TeamPtr> map;
	for (auto& team : m_teams)
		map[team->GetColour()] = std::move(team);

	m_teams.clear();
	for (Colour c : passOrder)
		m_teams.push_back(std::move(map[c]));
#else // Normal passing rule.
	for (auto it = m_teams.begin(); it != m_teams.end(); ++it)
		if ((*it)->GetColour() == passOrder.front())
		{
			std::rotate(m_teams.begin(), it, m_teams.end());
			break;
		}
#endif

	m_pPhase = PhasePtr(new UpkeepPhase(this));
}

int LiveGame::PushRecord(std::unique_ptr<Record> pRec)
{
	int id = m_nextRecordID++;
	pRec->SetID(id);
	m_records.push_back(std::move(pRec));
	return id;
}

RecordPtr LiveGame::PopRecord()
{
	size_t pop = GetLastPoppableRecord();

	VerifyModel("LiveGame::PopRecord 1", pop >= 0);

	RecordPtr pRec = std::move(m_records[pop]);
	m_records.erase(m_records.begin() + pop);
	return pRec;
}

int LiveGame::GetLastPoppableRecord() const
{
	int lastPoppable = (int)m_records.size() - 1;
	for (auto rec = m_records.rbegin(); rec != m_records.rend() && (*rec)->IsMessageRecord(); ++rec)
		--lastPoppable;

	return lastPoppable;
}

void LiveGame::ShipMovedFrom(const Hex& hex, Colour colour)
{
	GetActionPhase().ShipMovedFrom(hex, colour);
}

void LiveGame::ShipMovedTo(const Hex& hex, Colour colour) 
{
	GetActionPhase().ShipMovedTo(hex, colour);
}

LiveGame::LogVec LiveGame::GetLogs() const
{
	LogVec logs;
	for (auto& rec : m_records)
	{
		std::string msg = rec->GetMessage(*this);
		if (!msg.empty())
			logs.push_back(LogVec::value_type(rec->GetID(), msg));
	}
	return logs;
}

void LiveGame::Save() const
{
	std::ostringstream ss;
	ss << "data/games/live/" << m_id << ".xml";
	bool bOK = Serial::SaveClass(ss.str(), *this);
	ASSERT(bOK);
}

void LiveGame::Save(Serial::SaveNode& node) const 
{
	node.SaveCntr("records", m_records, Serial::ObjectSaver());
	node.SaveObject("phase", m_pPhase);
	node.SaveEnum("game_phase", m_gamePhase);
	node.SaveType("next_record_id", m_nextRecordID);
	__super::Save(node);
}

void LiveGame::Load(const Serial::LoadNode& node)
{
	node.LoadCntr("records", m_records, Serial::ObjectLoader());
	node.LoadObject("phase", m_pPhase);
	node.LoadEnum("game_phase", m_gamePhase);
	node.LoadType("next_record_id", m_nextRecordID);
	__super::Load(node);

	if (m_pPhase)
		m_pPhase->SetGame(*this);
}

DEFINE_ENUM_NAMES(LiveGame::GamePhase) { "Lobby", "ChooseTeam", "Main", "" };
