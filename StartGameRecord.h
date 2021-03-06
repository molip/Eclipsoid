#pragma once

#include "Record.h"
#include "Resources.h"
#include "Bag.h"

#include <map> 

enum class TechType;

class StartGameRecord : public Record
{
public:
	StartGameRecord();

	virtual void Save(Serial::SaveNode& node) const override;
	virtual void Load(const Serial::LoadNode& node) override;

private:
	virtual void Apply(bool bDo, const Game& game, GameState& gameState) override;
	virtual void Update(const Game& game, const RecordContext& context) const override;
	virtual std::string GetMessage(const Game& game) const override;

	EnumArray<HexRing, HexPile> m_hexPiles;
};
