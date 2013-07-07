#pragma once

#include "Resources.h"
#include "Xml.h"

#include <string>
#include <memory>

class Controller;
class Player;
class Game;
class LiveGame; 

namespace Input 
{

class Message
{
public:
	virtual ~Message() {}
	virtual bool Process(Controller& controller, Player& player) const { return true; }

protected:
	LiveGame& GetLiveGame(Player& player) const;
};

typedef std::unique_ptr<Message> MessagePtr;
MessagePtr CreateMessage(const std::string& xml);

//-----------------------------------------------------------------------------

struct Register: Message 
{
	Register(const Xml::Element& node);
	int GetPlayerID() const { return m_idPlayer; }
private:
	int m_idPlayer;
};

struct JoinGame: Message 
{
	JoinGame(const Xml::Element& node);
	virtual bool Process(Controller& controller, Player& player) const override; 
	int m_idGame;
};

struct ExitGame : Message // Returns player to game list - they're still in the game.
{
	virtual bool Process(Controller& controller, Player& player) const override; 
};

struct StartReview : Message 
{
	virtual bool Process(Controller& controller, Player& player) const override; 
};

struct ExitReview : Message 
{
	virtual bool Process(Controller& controller, Player& player) const override; 
};

struct AdvanceReview : Message 
{
	virtual bool Process(Controller& controller, Player& player) const override; 
};

struct RetreatReview : Message 
{
	virtual bool Process(Controller& controller, Player& player) const override; 
};

struct CreateGame : Message 
{
	virtual bool Process(Controller& controller, Player& player) const override; 
};

struct StartGame : Message 
{
	virtual bool Process(Controller& controller, Player& player) const override; 
};

struct ChooseTeam : Message 
{
	ChooseTeam(const Xml::Element& node);
	virtual bool Process(Controller& controller, Player& player) const override; 
	std::string m_race, m_colour;
};

struct StartAction : Message 
{
	StartAction(const Xml::Element& node);
	virtual bool Process(Controller& controller, Player& player) const override; 
	std::string m_action;
};

struct Undo : Message 
{
	virtual bool Process(Controller& controller, Player& player) const override; 
};

struct Commit : Message 
{
	virtual bool Process(Controller& controller, Player& player) const override; 
};

//-----------------------------------------------------------------------------

class CmdMessage : public Message
{
public:
	virtual bool Process(Controller& controller, Player& player) const;
};

struct CmdExplorePos : CmdMessage
{
	CmdExplorePos(const Xml::Element& node);
	int m_iPos;
};

struct CmdExploreHex : CmdMessage
{
	CmdExploreHex(const Xml::Element& node);
	int m_iRot;
	int m_iHex;
	bool m_bInfluence;
};

struct CmdExploreHexTake : CmdMessage {};

struct CmdExploreDiscovery : CmdMessage
{
	CmdExploreDiscovery(const Xml::Element& node);
};

struct CmdColonisePos : CmdMessage
{
	CmdColonisePos(const Xml::Element& node);
	int m_iPos;
};

struct CmdColoniseSquares : CmdMessage
{
	CmdColoniseSquares(const Xml::Element& node);
	
	Population m_fixed, m_grey, m_orbital;
};

struct CmdExploreReject : CmdMessage
{
};

struct CmdInfluenceSrc : CmdMessage
{
	CmdInfluenceSrc(const Xml::Element& node);
	int m_iPos;
};

struct CmdInfluenceDst : CmdMessage
{
	CmdInfluenceDst(const Xml::Element& node);
	int m_iPos;
};

struct CmdResearch : CmdMessage
{
	CmdResearch(const Xml::Element& node);
	int m_iTech;
};

struct CmdResearchArtifact : CmdMessage
{
	CmdResearchArtifact(const Xml::Element& node);
	Storage m_artifacts;
};

struct CmdMove : CmdMessage
{
	CmdMove(const Xml::Element& node);
};

struct CmdBuild : CmdMessage
{
	CmdBuild(const Xml::Element& node);
};

struct CmdDiplomacy : CmdMessage
{
	CmdDiplomacy(const Xml::Element& node);
};

struct CmdUpgrade : CmdMessage
{
	CmdUpgrade(const Xml::Element& node);
};

struct CmdTrade : CmdMessage
{
	CmdTrade(const Xml::Element& node);
};

}