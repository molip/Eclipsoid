#include "stdafx.h"
#include "ChooseTeamPhase.h"
#include "LiveGame.h"
#include "Output.h"
#include "Controller.h"
#include "StartGameRecord.h"
#include "CommitSession.h"
#include "Players.h"

ChooseTeamPhase::ChooseTeamPhase(LiveGame* pGame) : Phase(pGame)
{
}

void ChooseTeamPhase::AssignTeam(CommitSession& session, Player& player, RaceType race, Colour colour)
{
	LiveGame& game = GetGame();
	
	game.GetTeam(player).Assign(race, colour, game);

	AdvanceTurn();

	session.GetController().SendMessage(Output::ChooseTeam(game, false), player);

	bool allAssigned = true;
	for (auto& team : game.GetTeams())
		allAssigned &= team->GetColour() != Colour::None;
	
	if (allAssigned)
	{
		session.DoAndPushRecord(RecordPtr(new StartGameRecord));
		game.StartMainGamePhase(); // Deletes this.
		session.GetController().SendUpdateGame(game);
	}
	else
		UpdateClient(session.GetController(), nullptr);
}

const Team& ChooseTeamPhase::GetCurrentTeam() const
{
	return TurnPhase::GetCurrentTeam(GetGame());
}

void ChooseTeamPhase::UpdateClient(const Controller& controller, const Player* pPlayer) const
{
	controller.SendMessage(Output::ShowChoose(), GetGame(), pPlayer);
	controller.SendMessage(Output::UpdateChoose(GetGame()), GetGame(), pPlayer);

	const Player& currentPlayer = Players::Get(GetCurrentTeam().GetPlayerID());

	if (!pPlayer || pPlayer == &currentPlayer)
		controller.SendMessage(Output::ChooseTeam(GetGame(), true), currentPlayer);

	return;
}

void ChooseTeamPhase::Save(Serial::SaveNode& node) const
{
	Phase::Save(node);
	TurnPhase::Save(node);
}

void ChooseTeamPhase::Load(const Serial::LoadNode& node)
{
	Phase::Load(node);
	TurnPhase::Load(node);
}
REGISTER_DYNAMIC(ChooseTeamPhase)

