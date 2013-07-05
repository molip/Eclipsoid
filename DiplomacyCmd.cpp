#include "stdafx.h"
#include "DiplomacyCmd.h"
#include "Output.h"
#include "Input.h"
#include "Controller.h"
#include "LiveGame.h"
#include "Record.h"

DiplomacyCmd::DiplomacyCmd(Colour colour, LiveGame& game) : Cmd(colour)
{
}

void DiplomacyCmd::UpdateClient(const Controller& controller, const LiveGame& game) const
{
	controller.SendMessage(Output::ChooseDiplomacy(), GetPlayer(game));
}

CmdPtr DiplomacyCmd::Process(const Input::CmdMessage& msg, const Controller& controller, LiveGame& game)
{
	auto& m = CastThrow<const Input::CmdDiplomacy>(msg);
	
	return nullptr;
}

void DiplomacyCmd::Save(Serial::SaveNode& node) const 
{
	__super::Save(node);
}

void DiplomacyCmd::Load(const Serial::LoadNode& node) 
{
	__super::Load(node);
}

REGISTER_DYNAMIC(DiplomacyCmd)

//-----------------------------------------------------------------------------

class DiplomacyRecord : public Record
{
public:
	DiplomacyRecord() : m_colour(Colour::None) {}
	DiplomacyRecord(Colour colour) : m_colour(colour) {}

	virtual void Save(Serial::SaveNode& node) const override 
	{
	}
	
	virtual void Load(const Serial::LoadNode& node) override 
	{
	}

private:
	virtual void Apply(bool bDo, Game& game, const Controller& controller) override
	{
	}

	Colour m_colour;
};

REGISTER_DYNAMIC(DiplomacyRecord)