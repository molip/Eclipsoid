#pragma once

#include "Cmd.h"

class TradeCmd : public Cmd
{
public:
	TradeCmd() {}
	TradeCmd(Colour colour, LiveGame& game);

	virtual void UpdateClient(const Controller& controller, const LiveGame& game) const override;
	virtual CmdPtr Process(const Input::CmdMessage& msg, const Controller& controller, LiveGame& game) override;

	virtual void Save(Serial::SaveNode& node) const override;
	virtual void Load(const Serial::LoadNode& node) override;

private:
};

