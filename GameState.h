#pragma once

#include "Map.h"
#include "Bag.h"

#include <map>

class Game;
class Battle;
class TeamState;
class ShipBattle;
class PopulationBattle;

DEFINE_UNIQUE_PTR(Battle)
DEFINE_UNIQUE_PTR(TeamState)

class GameState
{
public:
	GameState(Game& game);
	~GameState();
	GameState(const GameState& rhs) = delete;
	GameState(const GameState& rhs, Game& game);
	bool operator==(const GameState& rhs) const;

	void Init(const Game& game);

	TeamState& GetTeamState(Colour c);
	const TeamState& GetTeamState(Colour c) const { return const_cast<GameState*>(this)->GetTeamState(c); }

	std::map<TechType, int>& GetTechnologies() { return m_techs; }
	Map& GetMap() { return m_map; }
	void ProduceTechnologies(int count, bool bDo);

	bool IncrementRound(bool bDo); // Returns true if game finished.
	Battle& GetBattle();
	void AttachBattle(BattlePtr battle);
	BattlePtr DetachBattle();
	ShipBattle& GetShipBattle();
	PopulationBattle& GetPopulationBattle();

	ReputationBag& GetReputationBag() { return m_repBag; }
	BagState<TechnologyBag>& GetTechnologyBag() { return m_techBagState; }
	BagState<DiscoveryBag>& GetDiscoveryBag() { return m_discBagState; }
	HexPile& GetHexPile(HexRing ring) { return m_hexPiles[ring]; }
	HexPile& GetHexDiscardPile(HexRing ring) { return m_hexDiscardPiles[ring]; }

	Hex& AddHex(const MapPos& pos, int id, int rotation);
	void DeleteHex(const MapPos& pos);

	void Save(Serial::SaveNode& node) const;
	void Load(const Serial::LoadNode& node);

	std::map<TechType, int> m_techs;
	Map	m_map; // After m_discBag
	int m_iRound;
	BattlePtr m_battle;

	std::map<Colour, TeamStatePtr> m_teamStates;

private:
	void InitBags(const Game& game);
		
	ReputationBag m_repBag;
	BagState<TechnologyBag> m_techBagState;
	BagState<DiscoveryBag> m_discBagState;
	EnumArray<HexRing, HexPile> m_hexPiles, m_hexDiscardPiles;
};
