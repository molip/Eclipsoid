#pragma once

#include "MovePopulationCommand.h"
#include "MapPos.h"
#include "Discovery.h"
#include "Hex.h"

#include <vector> 
#include <set> 

enum class Resource;

class ColoniseCmd : public Cmd
{
public:
	ColoniseCmd() {}
	ColoniseCmd(Colour colour, const LiveGame& game);

	virtual void UpdateClient(const Controller& controller, const LiveGame& game) const override;
	virtual ProcessResult Process(const Input::CmdMessage& msg, CommitSession& session) override;

	virtual void Save(Serial::SaveNode& node) const override;
	virtual void Load(const Serial::LoadNode& node) override;

private:
	std::vector<MapPos> GetPositions(const LiveGame& game) const;
};

class ColoniseSquaresCmd : public MovePopulationCommand
{
public:
	ColoniseSquaresCmd() {}
	ColoniseSquaresCmd(Colour colour, const LiveGame& game, const MapPos& pos);

	virtual void UpdateClient(const Controller& controller, const LiveGame& game) const override;
	virtual ProcessResult Process(const Input::CmdMessage& msg, CommitSession& session) override;

	virtual void Save(Serial::SaveNode& node) const override;
	virtual void Load(const Serial::LoadNode& node) override;

private:
	SquareCounts GetSquareCounts(const LiveGame& game) const;

	MapPos m_pos;
};
